const { app, BrowserWindow, ipcMain } = require('electron');
const path = require('path');
const mqtt = require("mqtt");
const sqlite3 = require("sqlite3").verbose();
const fs = require('fs');
const mqttClient = mqtt.connect('mqtt://10.0.0.70');
let win;
let status = {};
let sendData;
let topics = [];
let dataBuffer = {}; // 수집된 데이터를 저장할 객체로 변경
dataBuffer.status = 0;
// SQLite 데이터베이스 연결
const db = new sqlite3.Database('sensor_data.sqlite');
let insertDataInterval;

const toKoreanTime = (date) => {
    const utc = date.getTime() + (date.getTimezoneOffset() * 60000);
    const kst = new Date(utc + (9 * 3600000)); // UTC+9
    return kst.toLocaleString('ko-KR', { timeZone: 'Asia/Seoul' }).replace('T', ' ').substring(0, 23);
};


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

// MQTT 구독 및 메시지 수신 설정
function subscribeToMqtt() {
    
    mqttClient.subscribe('#', (err) => {
        if (err) {
            sendData = `구독 실패`;
            win.webContents.send('serial-data', sendData);
            console.error('Subscription error:', err);
            setTimeout(subscribeToMqtt, 5000); // 5초 후 다시 시도
        } else {
            sendData = `mqtt 구독 성공`;
            win.webContents.send('serial-data', sendData);
            console.log(`Subscribed to topic`);
        }
    });

    mqttClient.on('message', (topic, message) => {
        try {
            if (!topics.includes(topic)){
                sendData = `구독 topic 명 : ${topic}`;
                win.webContents.send('serial-data', sendData);
                topics.push(topic);
            }
            const textData = message.toString('utf-8');
            let data = textData.trim().split(' ');
            for(let i = 0; i<14; i++){
                let pose = data[i].split(":");
                if((pose[1] != -1)& pose[1] != "inf"){
                    dataBuffer[pose[0]] = Number(pose[1]);
                }
                
            }
            console.log(dataBuffer);
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
            if (!insertDataInterval) {
                insertDataInterval = setInterval(() => {
                    const now = new Date();
                    const time = new Date(now.getTime());
                    const timestamp = time.toISOString();
                    insertData(timestamp, topic, dataBuffer);
                }, 500);
            }
        } catch (e) {
            console.error('Error parsing message:', e);
        }
    });
}

// DB 연결 종료 시 처리
process.on('SIGINT', () => {
    db.close((err) => {
        if (err) {
            console.error('Error closing database:', err);
        }
        console.log('Database closed');
        process.exit(0);
    });
});


function createWindow() {
    win = new BrowserWindow({
        width: 800,
        height: 600,
        webPreferences: {
            preload: path.join(__dirname, 'preload.js'),
            contextIsolation: true,
            enableRemoteModule: false,
        }
    });

    win.loadFile('index.html');
    subscribeToMqtt();
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

ipcMain.on('checkOn', async (event) => {
    try {
        if(sendData != "data sending...."){
            event.sender.send('serial-status', 'mqtt 연결 실패');
        }else if(dataBuffer.status == 0){
            dataBuffer.status = 1;
            console.log("check start");
            event.sender.send('serial-status', '측정 시작');
        }else if(dataBuffer.status == 1){
            console.log("check already start");
            event.sender.send('serial-status', '측정 중..');
        }
    } catch (error) {
        event.sender.send('serial-status', '실패');
        console.error('Error connecting to serial port:', error);
    }
});

ipcMain.on('checkOff', async (event) => {
    try {
        if(sendData != "data sending...."){
            event.sender.send('serial-status', '연결 실패');
        }else if(dataBuffer.status == 1){
            dataBuffer.status = 0;
            console.log("check end");
            event.sender.send('serial-status', '측정 종료');
        }else if(dataBuffer.status == 0){
            console.log("check already end");
            event.sender.send('serial-status', '측정하지 않는 중...');
        }
    } catch (error) {
        event.sender.send('serial-status', '실패');
        console.error('Error connecting to serial port:', error);
    }
});