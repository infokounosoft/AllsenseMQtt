// renderer.js
// 수신된 데이터를 표시할 DOM 요소
const scrollBox = document.getElementById('scrollBox');

// 시리얼 데이터 수신 이벤트
window.electron.onSerialData((data) => {
    console.log('수신된 데이터:', data);
    
    // 새로운 데이터 항목을 생성
    const newDataElement = document.createElement('p');
    newDataElement.textContent = `${data}`;
    
    // 스크롤 박스에 추가
    scrollBox.appendChild(newDataElement);
    
    // 스크롤 박스의 스크롤을 가장 아래로 이동
    scrollBox.scrollTop = scrollBox.scrollHeight;
});

// 시리얼 포트 상태 수신 이벤트
window.electron.onSerialStatus((status) => {
    console.log('메인과 연결 상태:', status);
    
    // 상태 메시지를 스크롤 박스에 추가
    const statusElement = document.createElement('p');
    statusElement.textContent = `${status}`;
    
    // 스크롤 박스에 추가
    scrollBox.appendChild(statusElement);
    
    // 스크롤 박스의 스크롤을 가장 아래로 이동
    scrollBox.scrollTop = scrollBox.scrollHeight;
});

document.getElementById('startButton').addEventListener('click', () => {
    window.electron.checkOn();
});

document.getElementById('endButton').addEventListener('click', () => {
    window.electron.checkOff();
});

document.getElementById("submitButton").addEventListener("click", function() {
    const textValue = document.getElementById("textInput").value;
    window.electron.submit(textValue);
});