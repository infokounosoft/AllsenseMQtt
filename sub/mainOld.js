const express = require('express');
const app = express();
const http = require('http');
const server = http.createServer(app);
const mqtt = require('mqtt');
const { SerialPort } = require('serialport');
const { ReadlineParser } = require('serialport');
const topic = "jude";
const readline = require('readline');

const rl = readline.createInterface({
    input: process.stdin,
    output: process.stdout
});

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
                    if (error) {
                        console.log("연결된 포트 없음");
                        reject(error);
                    } else {
                        console.log("시리얼 포트 연결됨");
                        resolve(port);
                    }
                });
            });

            const parser = port.pipe(new ReadlineParser());

            // MQTT 브로커에 연결합니다.
            const client = mqtt.connect('mqtt://203.251.137.136');

            client.on('connect', function () {
                console.log('MQTT 클라이언트 연결됨');
                console.log("컨트롤 + S 키를 눌러 시작 시간 측정, 컨트롤 + E 키를 눌러 종료시간을 측정합니다.");

                let dataBuffer = {}; // 수집된 데이터를 저장할 객체로 변경
                dataBuffer.status = 0;
                rl.input.on('keypress', (char, key) => {
                    if (key.ctrl && key.name === 's') {
                        if(dataBuffer.status == 0){
                            console.log("시작 시간 측정");
                        }
                        if(dataBuffer.status == 1){
                            console.log("시작 시간이 이미 측정되었습니다.");
                            console.log("종료 시간을 측정해 주세요.");
                        }
                        dataBuffer.status = 1;
                    } else if (key.ctrl && key.name === 'e') {
                        if(dataBuffer.status == 1){
                            console.log("종료 시간 측정");
                        }
                        if(dataBuffer.status == 0){
                            console.log("시작 시간을 측정하세요");
                        }
                        dataBuffer.status = 0;

                    }
                });
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
                            }else{
                                dataBuffer[newValue1[0]] = Number(newValue1[1]); // 업데이트
                            }
                            
                        }
                    }
                });

                // 0.5초마다 수집된 데이터 publish
                setInterval(() => {
                    if (Object.keys(dataBuffer).length > 0) { // 데이터가 있을 경우에만
                        client.publish(topic, JSON.stringify(dataBuffer));
                        console.log(`Sent data to MQTT: ${JSON.stringify(dataBuffer)}`);
                    }
                }, 500);
            });

            return port;
        } else {
            console.log('No matching serial ports found.');
            return null;
        }
    } catch (error) {
        console.error('Error connecting to serial port:', error);
        return null;
    }
}

console.log('Hello');

setTimeout(() => {
    console.log('Start');
    connectSerialPort();
}, 500);
const { app, BrowserWindow, ipcMain } = require('electron');

let mainWindow;

function createWindow() {
    mainWindow = new BrowserWindow({
        width: 800,
        height: 600,
        webPreferences: {
            contextIsolation: true,
            enableRemoteModule: false,
            preload: __dirname + '/preload.js' // preload.js 파일을 사용
        }
    });

    mainWindow.loadFile('index.html');
}

app.whenReady().then(createWindow);

ipcMain.on('update-variable', (event, newValue) => {
    // 여기서 index.js의 변수를 변경
    console.log("변경된 변수 값:", newValue);
});

// const { app, BrowserWindow } = require('electron');
// const path = require('path');
 
// const createWindow = () => {
//     const win = new BrowserWindow({
//         width: 640,
//         height: 480,
//         webPreferences: { preload: path.join(__dirname, 'preload.js') }
//     });
 
//     win.loadFile('index.html');
// };
 
// app.whenReady().then(() => {
//     createWindow();

//     app.on('activate', () => {
//         if (BrowserWindow.getAllWindows().length === 0) createWindow();
//     });
// });
 
// app.on('window-all-closed', () => {
//     if (process.platform !== 'darwin') app.quit();
// });