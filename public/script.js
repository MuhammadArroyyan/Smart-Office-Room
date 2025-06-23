// Import fungsi-fungsi yang kita butuhkan dari Firebase SDK
import { initializeApp } from "https://www.gstatic.com/firebasejs/9.22.2/firebase-app.js";
import { getDatabase, ref, onValue } from "https://www.gstatic.com/firebasejs/9.22.2/firebase-database.js";

// =================================================================
// === BAGIAN 1: KONFIGURASI FIREBASE ===
// =================================================================
const firebaseConfig = {
    apiKey: "AIzaSyCL01ZczH3DV1pDL0drOTmnKgukk-YvJuk",
    authDomain: "smart-office-room-23.firebaseapp.com",
    databaseURL: "https://smart-office-room-23-default-rtdb.asia-southeast1.firebasedatabase.app",
    projectId: "smart-office-room-23",
    storageBucket: "smart-office-room-23.appspot.com",
    messagingSenderId: "39174489550",
    appId: "1:39174489550:web:8e6cfe7a8d8a8edf35179f"
};

// Inisialisasi Firebase
const app = initializeApp(firebaseConfig);
const db = getDatabase(app);


// =================================================================
// === BAGIAN 2: LOGIKA APLIKASI WEB ===
// =================================================================

// === Threshold (Harus sesuai dengan di ESP32) ===
const DISTANCE_THRESHOLD = 50; 
const HUMIDITY_THRESHOLD = 70; // Pastikan ini sama dengan di ESP32 Anda
const LDR_THRESHOLD = 1000;

// === Elemen UI ===
const lastUpdatedEl = document.getElementById("last-updated");
const tempBlock = document.getElementById("temp-block");
const humBlock = document.getElementById("hum-block");
const distBlock = document.getElementById("dist-block");
const ldrBlock = document.getElementById("ldr-block");
const tempEl = document.getElementById("temp");
const tempGaugeEl = document.getElementById("temp-gauge");
const tempStatusEl = document.getElementById("tempStatus");
const humEl = document.getElementById("hum");
const humGaugeEl = document.getElementById("hum-gauge");
const humStatusEl = document.getElementById("humStatus");
const distEl = document.getElementById("dist");
const distStatusEl = document.getElementById("distStatus");
const ldrEl = document.getElementById("ldr");
const ldrStatusEl = document.getElementById("ldrStatus");

// === Listener Data dari Firebase ===
const sensorRef = ref(db, 'sensor/'); // Path ini harus sama persis dengan di ESP32

onValue(sensorRef, (snapshot) => {
    const data = snapshot.val();
    if (data) {
        updateUI(data);
        // Panggil feather.replace() di sini agar ikon dirender ulang saat ada pembaruan
        feather.replace(); 
    } else {
        lastUpdatedEl.textContent = "Menunggu data dari sensor...";
    }
});

function updateUI(data) {
    // Update timestamp
    const now = new Date();
    lastUpdatedEl.textContent = `Terakhir Diperbarui: ${now.toLocaleTimeString('id-ID')}`;

    // 1. Update Suhu
    if (data.temperature !== undefined) {
        tempEl.textContent = data.temperature.toFixed(1);
        const tempRotation = (data.temperature / 50) * 180;
        tempGaugeEl.style.setProperty('--gauge-value', `${Math.min(tempRotation, 180)}deg`);
        
        tempStatusEl.textContent = "Status normal";
        tempStatusEl.classList.remove("warning");
        tempBlock.classList.remove("warning");
    }

    // 2. Update Kelembapan
    if (data.humidity !== undefined) {
        humEl.textContent = data.humidity.toFixed(0);
        const humRotation = (data.humidity / 100) * 180;
        humGaugeEl.style.setProperty('--gauge-value', `${Math.min(humRotation, 180)}deg`);

        if (data.humidity > HUMIDITY_THRESHOLD) {
            humStatusEl.textContent = "Kelembapan tinggi!";
            humStatusEl.classList.add("warning");
            humBlock.classList.add("warning");
        } else {
            humStatusEl.textContent = "Kelembapan normal";
            humStatusEl.classList.remove("warning");
            humBlock.classList.remove("warning");
        }
    }

    // 3. Update Jarak (Ultrasonik)
    if (data.distance !== undefined) {
        distEl.textContent = data.distance.toFixed(0);
        if (data.distance < DISTANCE_THRESHOLD) {
            distStatusEl.textContent = "Orang terdeteksi masuk!";
            distStatusEl.classList.add("warning");
            distBlock.classList.add("warning");
        } else {
            distStatusEl.textContent = "Tidak ada orang terdeteksi";
            distStatusEl.classList.remove("warning");
            distBlock.classList.remove("warning");
        }
    }

    // 4. Update Cahaya (LDR)
    if (data.ldr !== undefined) {
        ldrEl.textContent = data.ldr;
        if (data.ldr < LDR_THRESHOLD) {
            ldrStatusEl.textContent = "Ruangan gelap, lampu menyala";
            ldrStatusEl.classList.add("warning");
            ldrBlock.classList.add("warning");
        } else {
            ldrStatusEl.textContent = "Cahaya cukup";
            ldrStatusEl.classList.remove("warning");
            ldrBlock.classList.remove("warning");
        }
    }
}

// Inisialisasi ikon saat halaman pertama kali dimuat
feather.replace();