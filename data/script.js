// ==================== WEBSOCKET ====================
var gateway = `ws://${window.location.hostname}/ws`;
//var websocket;

// ==================== QUẢN LÝ TRẠNG THÁI (NAMESPACE) ====================
const AppState = {
    websocket: null,
    gateway: `ws://${window.location.hostname}/ws`,
    gaugeTemp: null,
    gaugeHumi: null
};

window.addEventListener('load', onLoad);

function onLoad(event) {
    initWebSocket();
}

function onOpen(event) {
    console.log('Connection opened');
}

function onClose(event) {
    console.log('Connection closed');
    setTimeout(initWebSocket, 2000);
}

function initWebSocket() {
    // console.log('Trying to open a WebSocket connection…');
    // websocket = new WebSocket(gateway);
    // websocket.onopen = onOpen;
    // websocket.onclose = onClose;
    // websocket.onmessage = onMessage;

    console.log('Trying to open a WebSocket connection…');
    // TRỎ VÀO APPSTATE
    AppState.websocket = new WebSocket(AppState.gateway);
    AppState.websocket.onopen = onOpen;
    AppState.websocket.onclose = onClose;
    AppState.websocket.onmessage = onMessage;
}

function Send_Data(data) {
    // if (websocket && websocket.readyState === WebSocket.OPEN) {
    //     websocket.send(data);
    //     console.log("📤 Gửi:", data);
    // } else {
    //     console.warn("⚠️ WebSocket chưa sẵn sàng!");
    //     alert("⚠️ WebSocket chưa kết nối!");
    // }

    // TRỎ VÀO APPSTATE
    if (AppState.websocket && AppState.websocket.readyState === WebSocket.OPEN) {
        AppState.websocket.send(data);
        console.log("📤 Gửi:", data);
    } else {
        console.warn("⚠️ WebSocket chưa sẵn sàng!");
        alert("⚠️ WebSocket chưa kết nối!");
    }
}

function onMessage(event) {
    // console.log("📩 Nhận:", event.data);
    // try {
    //     var data = JSON.parse(event.data);
    //     // Có thể thêm xử lý riêng nếu cần (ví dụ cập nhật trạng thái)
    // } catch (e) {
    //     console.warn("Không phải JSON hợp lệ:", event.data);
    // }
console.log("📩 Nhận:", event.data);
    try {
        var data = JSON.parse(event.data);
        
        // Gọi đồng hồ từ AppState ra để vẽ số liệu mới
        if (data.temperature !== undefined && AppState.gaugeTemp) {
            AppState.gaugeTemp.refresh(data.temperature);
        }
        if (data.humidity !== undefined && AppState.gaugeHumi) {
            AppState.gaugeHumi.refresh(data.humidity);
        }

        // BỔ SUNG: Xử lý hiển thị cho Tab Thông tin
        if (data.page === "info") {
            document.getElementById("infoSsid").innerText = data.ssid;
            document.getElementById("infoIp").innerText = data.ip;
            document.getElementById("infoMac").innerText = data.mac;
            document.getElementById("infoRam").innerText = data.ram + " KB";
            document.getElementById("infoUptime").innerText = data.uptime + " giây";
        }
    } catch (e) {
        console.warn("Không phải JSON hợp lệ:", event.data);
    }
}


// ==================== UI NAVIGATION ====================
let relayList = [];
let deleteTarget = null;

function showSection(id, event) {
    document.querySelectorAll('.section').forEach(sec => sec.style.display = 'none');
    document.getElementById(id).style.display = id === 'settings' ? 'flex' : 'block';
    document.querySelectorAll('.nav-item').forEach(i => i.classList.remove('active'));
    event.currentTarget.classList.add('active');
}


// ==================== HOME GAUGES ====================
window.onload = function () {
    // const gaugeTemp = new JustGage({
    //     id: "gauge_temp",
    //     value: 26,
    //     min: -10,
    //     max: 50,
    //     donut: true,
    //     pointer: false,
    //     gaugeWidthScale: 0.25,
    //     gaugeColor: "transparent",
    //     levelColorsGradient: true,
    //     levelColors: ["#00BCD4", "#4CAF50", "#FFC107", "#F44336"]
    // });

    // const gaugeHumi = new JustGage({
    //     id: "gauge_humi",
    //     value: 60,
    //     min: 0,
    //     max: 100,
    //     donut: true,
    //     pointer: false,
    //     gaugeWidthScale: 0.25,
    //     gaugeColor: "transparent",
    //     levelColorsGradient: true,
    //     levelColors: ["#42A5F5", "#00BCD4", "#0288D1"]
    // });

    // Lưu thẳng vào AppState, không tạo biến mới
    AppState.gaugeTemp = new JustGage({
        id: "gauge_temp",
        value: 0,
        min: -10,
        max: 50,
        decimals: 1,
        donut: true,
        pointer: false,
        gaugeWidthScale: 0.25,
        gaugeColor: "transparent",
        levelColorsGradient: true,
        levelColors: ["#00BCD4", "#4CAF50", "#FFC107", "#F44336"]
    });

    AppState.gaugeHumi = new JustGage({
        id: "gauge_humi",
        value: 0,
        min: 0,
        max: 100,
        decimals: 1,
        donut: true,
        pointer: false,
        gaugeWidthScale: 0.25,
        gaugeColor: "transparent",
        levelColorsGradient: true,
        levelColors: ["#42A5F5", "#00BCD4", "#0288D1"]
    });

    // setInterval(() => {
    //     gaugeTemp.refresh(Math.floor(Math.random() * 15) + 20);
    //     gaugeHumi.refresh(Math.floor(Math.random() * 40) + 40);
    // }, 3000);
};


// ==================== DEVICE FUNCTIONS ====================
// function openAddRelayDialog() {
//     document.getElementById('addRelayDialog').style.display = 'flex';
// }
// function closeAddRelayDialog() {
//     document.getElementById('addRelayDialog').style.display = 'none';
// }
// function saveRelay() {
//     const name = document.getElementById('relayName').value.trim();
//     const gpio = document.getElementById('relayGPIO').value.trim();
//     if (!name || !gpio) return alert("⚠️ Please fill all fields!");
//     relayList.push({ id: Date.now(), name, gpio, state: false });
//     renderRelays();
//     closeAddRelayDialog();
// }
// function renderRelays() {
//     const container = document.getElementById('relayContainer');
//     container.innerHTML = "";
//     relayList.forEach(r => {
//         const card = document.createElement('div');
//         card.className = 'device-card';
//         card.innerHTML = `
//       <i class="fa-solid fa-bolt device-icon"></i>
//       <h3>${r.name}</h3>
//       <p>GPIO: ${r.gpio}</p>
//       <button class="toggle-btn ${r.state ? 'on' : ''}" onclick="toggleRelay(${r.id})">
//         ${r.state ? 'ON' : 'OFF'}
//       </button>
//       <i class="fa-solid fa-trash delete-icon" onclick="showDeleteDialog(${r.id})"></i>
//     `;
//         container.appendChild(card);
//     });
// }
// function toggleRelay(id) {
//     const relay = relayList.find(r => r.id === id);
//     if (relay) {
//         relay.state = !relay.state;
//         const relayJSON = JSON.stringify({
//             page: "device",
//             value: {
//                 name: relay.name,
//                 status: relay.state ? "ON" : "OFF",
//                 gpio: relay.gpio
//             }
//         });
//         Send_Data(relayJSON);
//         renderRelays();
//     }
// }
// function showDeleteDialog(id) {
//     deleteTarget = id;
//     document.getElementById('confirmDeleteDialog').style.display = 'flex';
// }
// function closeConfirmDelete() {
//     document.getElementById('confirmDeleteDialog').style.display = 'none';
// }
// function confirmDelete() {
//     relayList = relayList.filter(r => r.id !== deleteTarget);
//     renderRelays();
//     closeConfirmDelete();
// }


// ==================== SETTINGS FORM (BỔ SUNG) ====================
// document.getElementById("settingsForm").addEventListener("submit", function (e) {
//     e.preventDefault();

//     const ssid = document.getElementById("ssid").value.trim();
//     const password = document.getElementById("password").value.trim();
//     const token = document.getElementById("token").value.trim();
//     const server = document.getElementById("server").value.trim();
//     const port = document.getElementById("port").value.trim();

//     const settingsJSON = JSON.stringify({
//         page: "setting",
//         value: {
//             ssid: ssid,
//             password: password,
//             token: token,
//             server: server,
//             port: port
//         }
//     });

//     Send_Data(settingsJSON);
//     alert("✅ Cấu hình đã được gửi đến thiết bị!");
// });

// ==================== DEVICE CONTROL FUNCTIONS ====================

// Gửi độ sáng NEO Pixel (0-255)
function sendNeoBrightness(value) {
    document.getElementById("neoValue").innerText = value;
    
    const neoJSON = JSON.stringify({
        page: "device",
        value: {
            name: "NEO",
            action: "brightness",
            level: parseInt(value)
        }
    });
    Send_Data(neoJSON);
}

// Gửi trạng thái GPIO 18 (Bật/Tắt)
function sendGpioState(isChecked) {
    const state = isChecked ? 1 : 0;
    
    const gpioJSON = JSON.stringify({
        page: "device",
        value: {
            name: "GPIO18",
            action: "toggle",
            state: state
        }
    });
    Send_Data(gpioJSON);
}

// ==================== SETTINGS FORM ====================
// document.getElementById("settingsForm").addEventListener("submit", function (e) {
//     e.preventDefault();

//     const settingsJSON = JSON.stringify({
//         page: "setting",
//         value: {
//             ssid: document.getElementById("ssid").value.trim(),
//             password: document.getElementById("password").value.trim(),
//             token: document.getElementById("token").value.trim(),
//             server: document.getElementById("server").value.trim(),
//             port: document.getElementById("port").value.trim()
//         }
//     });

//     Send_Data(settingsJSON);
//     alert("✅ Cấu hình đã được gửi xuống ESP32!");
// });
document.getElementById("settingsForm").addEventListener("submit", function (e) {
    e.preventDefault();

    const settingsJSON = JSON.stringify({
        page: "setting",
        value: {
            ssid: document.getElementById("ssid").value.trim(),
            password: document.getElementById("password").value.trim(),
            deviceId: document.getElementById("deviceId").value.trim(), 
            token: document.getElementById("token").value.trim(),
            server: document.getElementById("server").value.trim(),
            port: document.getElementById("port").value.trim()
        }
    });

    Send_Data(settingsJSON);
    alert("✅ Cấu hình đã được gửi xuống ESP32!");
});

// ==================== ĐIỀU KHIỂN THIẾT BỊ ====================

function sendModeState(isManual) {
    const modeText = document.getElementById("modeStatus");
    const neoCard = document.getElementById("neoCard");
    const servoCard = document.getElementById("servoCard");

    if (isManual) {
        modeText.innerText = "MANUAL";
        modeText.style.color = "#F44336";
        // Mở khóa các nút điều khiển thủ công
        neoCard.style.opacity = "1";
        neoCard.style.pointerEvents = "auto";
        servoCard.style.opacity = "1";
        servoCard.style.pointerEvents = "auto";
    } else {
        modeText.innerText = "AUTO";
        modeText.style.color = "#9C27B0";
        // Khóa các nút điều khiển thủ công
        neoCard.style.opacity = "0.5";
        neoCard.style.pointerEvents = "none";
        servoCard.style.opacity = "0.5";
        servoCard.style.pointerEvents = "none";
    }

    const data = JSON.stringify({
        page: "device_control",
        type: "mode",
        value: isManual ? 0 : 1 // 1 = Auto, 0 = Manual
    });
    Send_Data(data);
}

function sendNeoBrightness(value) {
    document.getElementById("neoValue").innerText = value;
    const data = JSON.stringify({
        page: "device_control",
        type: "neo_pwm",
        value: parseInt(value)
    });
    Send_Data(data);
}

function sendServoState(isOn) {
    const data = JSON.stringify({
        page: "device_control",
        type: "servo_angle",
        value: isOn ? 180 : 0 // Bật = quay 180 độ, Tắt = quay về 0 độ
    });
    Send_Data(data);
}