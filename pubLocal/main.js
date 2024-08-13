const { app, BrowserWindow, ipcMain } = require('electron');
const { SerialPort } = require('serialport');
const path = require('path');
const { ReadlineParser } = require('serialport');
const sqlite3 = require("sqlite3").verbose();
const fs = require('fs');

let dataBuffer = {}; // 수집된 데이터를 저장할 객체로 변경
dataBuffer.status = 0;
let win;
let sendData;
let open;
let status = {};
let topics = [];

/////////////////
//함수 설정
/////////////////

const toKoreanTime = (date) => {
    const utc = date.getTime() + (date.getTimezoneOffset() * 60000);
    const kst = new Date(utc + (9 * 3600000)); // UTC+9
    return kst.toLocaleString('ko-KR', { timeZone: 'Asia/Seoul' }).replace('T', ' ').substring(0, 23);
};

// 데이터베이스에 데이터 삽입하는 함수
function insertData(timestamp, topic, data) {
    db.run(
        `
        INSERT INTO sensor_data (timestamp, topic, status, CH2O, MQ131, VOC, NH3, CO, NO2, CO2, Temperature, Humidity, C3H8, C4H10, CH4, H2, C2H5OH)
        VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)
        `,
        [
            timestamp,
            topic,
            data.status,
            data.CH2O,
            data.MQ131,
            data.VOC,
            data.NH3,
            data.CO,
            data.NO2,
            data.CO2,
            data.Temperature,
            data.Humidity,
            data.C3H8,  
            data.C4H10,  
            data.CH4,    
            data.H2,     
            data.C2H5OH 
        ],
        (err) => {
            if (err) {
                if (err.code === 'SQLITE_BUSY') {
                    console.error('Database busy, retrying...');
                    setTimeout(() => insertData(timestamp, topic, data), 1000); // 1초 후 재시도
                } else {
                    console.error('Database insertion error:', err);
                }
            } else {
                // console.log('Data saved to SQLite');
            }
        }
    );
}

async function connectSerialPort() {
    try {
        const ports = await SerialPort.list();
        const targetVendorId1 = "1A86"; // 찾고자 하는 vendorId
        const targetVendorId2 = "1a86"; // 찾고자 하는 vendorId
        const targetProductId = "7523"; // 찾고자 하는 productId

        const connectedPort = ports.find(port => (port.vendorId === targetVendorId1 || port.vendorId === targetVendorId2) && port.productId === targetProductId);
        if (connectedPort) {

            const port = new SerialPort({
                path: connectedPort.path,
                baudRate: 115200,
                autoOpen: false
            });

            await new Promise((resolve, reject) => {
                port.open((error) => {
                    if(open == 1){
                        console.log("포트가 이미 연결되어 있습니다.");
                        // 렌더러 프로세스에 'serial-data' 이벤트 송신
                        win.webContents.send('serial-data', "포트가 이미 연결되어 있습니다.");
                        return resolve(port); // 함수를 종료하고 현재 포트를 반환
                    }
                    if (error) {
                        sendData = "연결된 포트 없음"; // 예시 데이터
                        // 렌더러 프로세스에 'serial-data' 이벤트 송신
                        win.webContents.send('serial-data', sendData);
                        sendData = "연결된 포트 없음"; // 예시 데이터
                        // 렌더러 프로세스에 'serial-data' 이벤트 송신
                        win.webContents.send('serial-data', error);
                        console.log("non connectied port");
                        console.log(error)
                        reject(error);
                    } else {
                        sendData = "시리얼 포트 연결됨"; // 예시 데이터
                        // 렌더러 프로세스에 'serial-data' 이벤트 송신
                        win.webContents.send('serial-data', sendData);
                        console.log("port sink on");
                        open = 1;
                        resolve(port);
                    }
                });
            });

            const parser = port.pipe(new ReadlineParser());

            parser.on('data', function (data) {
                let value = data.trim().split('-');
                
                value[0] = value[0].trimEnd();
                if (value[0] == "BH1750" || value[0] == "PM2008") {
                    return; // 무시
                } else if (value[0].trim().split(':')[0] == "## SEND_BT_SENSORF2") {
                    return; // 무시
                }
            
                let newKey = value[0];
                let newValue = value[1];

                if (newKey == "CH2O") {
                    newValue = newValue.trim().split(" ");
                    dataBuffer[newKey] = Number(newValue[0]); // 업데이트
                } else if (newKey == "VOC" || newKey == "MQ131") {
                    dataBuffer[newKey] = Number(newValue); // 업데이트
                } else if (newKey == "SHT41") {
                    let newValue1 = newValue.trim().split('\t');
                    let newValue2 = newValue1[0].trim().split(':');
                    let temperature = Number(newValue2[1]);
                    newValue2 = newValue1[1].trim().split(':');
                    let humidity = Number(newValue2[1]);
                    dataBuffer.Temperature = temperature; // 업데이트
                    dataBuffer.Humidity = humidity; // 업데이트
                } else if (newKey == "SCD40") {
                    let newValue1 = newValue.trim().split('\t');
                    let co2 = Number(newValue1[0].trim().split(':')[1]);
                    dataBuffer.CO2 = co2; // 업데이트
                } else if (newKey == "MICS") {
                    let newValue1 = newValue.trim().split(':');
                    newValue1[0] = newValue1[0].trim();
                    if(newValue1[1] < 30000){
                        if (newValue1[0] == "C3H8"||newValue1[0] == "C4H10"||newValue1[0] == "CH4"){
                            if(newValue1[1] > 3000){
                                dataBuffer[newValue1[0]] = Number(newValue1[1]); // 업데이트
                            }
                        }else if(newValue1[1] !=0){
                            dataBuffer[newValue1[0]] = Number(newValue1[1]); // 업데이트
                        }
                        
                    }
                }
            });

            // 0.5초마다 수집된 데이터 publish
            setInterval(() => {
                if (Object.keys(dataBuffer).length > 0) { // 데이터가 있을 경우에만
                    if (!topics.includes(topic)){
                        sendData = `구독 topic 명 : ${topic}`;
                        win.webContents.send('serial-data', sendData);
                        topics.push(topic);
                    }
                    const data = dataBuffer;
                    const now = new Date();
                    const time = new Date(now.getTime());
                    const timestamp = time.toISOString();
                    
                    if (status[topic] == undefined){
                        status[topic] = 0
                    }
                    
                    if (data.status == 1 && status[topic]==0){
                        status[topic] = 1;
                        sendData = `${topic} 측정 시작 시간: ${toKoreanTime(now)}\n`;
                        win.webContents.send('serial-data', sendData);
                        fs.appendFileSync('측정시간.txt', `${topic} 측정 시작 시간: ${toKoreanTime(now)}\n`);
                        console.log("측정이 시작되었습니다.");
                    }else if(data.status == 0 && status[topic]==1){
                        status[topic] = 0;
                        sendData = `${topic} 측정 종료 시간: ${toKoreanTime(now)}\n`;
                        win.webContents.send('serial-data', sendData);
                        fs.appendFileSync('측정시간.txt', `${topic} 측정 종료 시간: ${toKoreanTime(now)}\n`);
                        console.log("측정이 종료되었습니다.");
                    }
                    // 데이터 삽입
                    insertData(timestamp, topic, data);
                    console.log(`Data saved in sqlite : ${data}`);
                }
            }, 500);

            return port;
        } else {
            sendData = "연결된 포트가 없습니다 포트를 연결하고 재실행 해주세요."; // 예시 데이터
            // 렌더러 프로세스에 'serial-data' 이벤트 송신
            win.webContents.send('serial-data', sendData);
            console.log('No matching serial ports found.');
            return null;
        }
    } catch (error) {
        sendData = "시리얼 포트와 연결 실패."; // 예시 데이터
        // 렌더러 프로세스에 'serial-data' 이벤트 송신
        win.webContents.send('serial-data', sendData);
        console.error('Error connecting to serial port:', error);
        return null;
    }
}

/////////////////
//함수 설정 끝
/////////////////

/////////////////////////////////////////
//DB 연결
/////////////////////////////////////////

// SQLite 데이터베이스 연결
const db = new sqlite3.Database('sensor_data.sqlite');

// 데이터베이스 테이블 생성
db.run(`
    CREATE TABLE IF NOT EXISTS sensor_data (
        id INTEGER PRIMARY KEY AUTOINCREMENT,
        timestamp TEXT,
        topic TEXT,
        status FLOAT,
        CH2O FLOAT,
        MQ131 FLOAT,
        VOC FLOAT,
        NH3 FLOAT,
        CO FLOAT,
        NO2 FLOAT,
        CO2 FLOAT,
        Temperature FLOAT,
        Humidity FLOAT,
        C3H8 FLOAT,    
        C4H10 FLOAT,   
        CH4 FLOAT,     
        H2 FLOAT,      
        C2H5OH FLOAT   
    )
`);

process.on('SIGINT', () => {
    db.close((err) => {
        if (err) {
            console.error('Error closing database:', err);
        }
        console.log('Database closed');
        process.exit(0);
    });
});

/////////////////////////////////////////
//DB 연결 끝
/////////////////////////////////////////

///////////////////////////////
// 창 만들기
///////////////////////////////

function createWindow() {
    win = new BrowserWindow({
        width: 1080,
        height: 920,
        webPreferences: {
            preload: path.join(__dirname, 'preload.js'),
            contextIsolation: true,
            enableRemoteModule: false,
        }
    });

    win.loadFile('index.html');
}

app.whenReady().then(() => {
    createWindow();
    app.on('activate', () => {
        if (BrowserWindow.getAllWindows().length === 0) createWindow();
    });
});

app.on('window-all-closed', () => {
    if (process.platform !== 'darwin') app.quit();
});

///////////////////////////////
// 창 만들기 끝
///////////////////////////////

///////////////////////////////
// electron 메인 렌더러 통신설정
///////////////////////////////

ipcMain.on('checkOn', async (event) => {
    try {
        if(dataBuffer.status == 0){
            dataBuffer.status = 1;
            console.log("check start");
            event.sender.send('serial-status', '측정 시작');
        }else if(dataBuffer.status == 1){
            console.log("check already start");
            event.sender.send('serial-status', '측정 중..');
        }
    } catch (error) {
        event.sender.send('serial-status', '메인과 통신 실패');
        console.error('Error connecting to serial port:', error);
    }
});

ipcMain.on('checkOff', async (event) => {
    try {
        if(dataBuffer.status == 1){
            dataBuffer.status = 0;
            console.log("check end");
            event.sender.send('serial-status', '측정 종료');
        }else if(dataBuffer.status == 0){
            console.log("check already end");
            event.sender.send('serial-status', '측정하지 않는 중...');
        }
    } catch (error) {
        event.sender.send('serial-status', '메인과 통신 실패');
        console.error('Error connecting to serial port:', error);
    }
});

ipcMain.on('submit', async (event,text) => {
    try {
        topic = text
        event.sender.send('serial-status', 'topic 설정 완료');
        console.log("connect error with main : topic")
        setTimeout(connectSerialPort, 500);
    }catch (error) {
        event.sender.send('serial-status', 'topic 설정 실패');
        console.error('Error connecting to main:', error);
    }
});

///////////////////////////////
// electron 메인 렌더러 통신설정 끝
///////////////////////////////