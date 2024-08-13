const { contextBridge, ipcRenderer } = require('electron');

contextBridge.exposeInMainWorld('electron', {
    checkOn: () => ipcRenderer.send('checkOn'),
    checkOff: () => ipcRenderer.send('checkOff'),
    submit: (text) => ipcRenderer.send('submit',text),
    onSerialData: (callback) => ipcRenderer.on('serial-data', (event, data) => callback(data)),
    onSerialStatus: (callback) => ipcRenderer.on('serial-status', (event, status) => callback(status))
});
