// PERBAIKAN: Gunakan sintaks Firebase v9 (modular)
import { initializeApp } from "https://www.gstatic.com/firebasejs/9.22.2/firebase-app.js";
import { getDatabase, ref, onValue } from "https://www.gstatic.com/firebasejs/9.22.2/firebase-database.js";

// =================================================================
// === KONFIGURASI FIREBASE (HARUS SAMA DENGAN DI ESP32) ===
// =================================================================
const firebaseConfig = {
    // PERINGATAN: GANTI SEMUA INI DENGAN KONFIGURASI FIREBASE ANDA
    apiKey: "GANTI_DENGAN_API_KEY_ANDA",
    authDomain: "proyek-anda.firebaseapp.com",
    databaseURL: "GANTI_DENGAN_DATABASE_URL_ANDA",
    projectId: "proyek-anda",
    storageBucket: "proyek-anda.appspot.com",
    messagingSenderId: "GANTI_DENGAN_SENDER_ID_ANDA",
    appId: "GANTI_DENGAN_APP_ID_ANDA"
};

// Inisialisasi Firebase
const app = initializeApp(firebaseConfig);
const db = getDatabase(app);

// === Threshold (Harus sesuai dengan di ESP32) ===
const DISTANCE_THRESHOLD = 50; // Jarak dalam cm untuk deteksi orang
const HUMIDITY_THRESHOLD = 60; // Kelembapan dalam %
const LDR_THRESHOLD = 1000;    // Nilai LDR untuk ruangan gelap

// === Elemen UI ===
const tempEl = document.getElementById("temp");
const tempStatusEl = document.getElementById("tempStatus");
const humEl = document.getElementById("hum");
const humStatusEl = document.getElementById("humStatus");
const distEl = document.getElementById("dist");
const distStatusEl = document.getElementById("distStatus");
const ldrEl = document.getElementById("ldr");
const ldrStatusEl = document.getElementById("ldrStatus");

// === Listener Data dari Firebase ===
// Path ini harus sama dengan yang diatur di ESP32
const sensorRef = ref(db, 'sensor/');

onValue(sensorRef, (snapshot) => {
    const data = snapshot.val();
    if (data) {
        console.log("Data diterima dari Firebase:", data); // Untuk debugging
        updateUI(data);
    } else {
        console.log("Tidak ada data di Firebase path.");
    }
});

function updateUI(data) {
    // 1. Update Suhu
    if (data.temperature !== undefined) {
        tempEl.textContent = `Suhu: ${data.temperature.toFixed(1)} °C`;
        // Anda bisa menambahkan logika status suhu jika perlu
        tempStatusEl.textContent = "Status normal";
        tempStatusEl.classList.remove("warning");
    }

    // 2. Update Kelembapan
    if (data.humidity !== undefined) {
        humEl.textContent = `Kelembapan: ${data.humidity.toFixed(0)} %`;
        if (data.humidity > HUMIDITY_THRESHOLD) {
            humStatusEl.textContent = "Kelembapan tinggi!";
            humStatusEl.classList.add("warning");
        } else {
            humStatusEl.textContent = "Kelembapan normal";
            humStatusEl.classList.remove("warning");
        }
    }

    // 3. Update Jarak (Ultrasonik)
    if (data.distance !== undefined) {
        distEl.textContent = `Jarak Pintu: ${data.distance.toFixed(0)} cm`;
        if (data.distance < DISTANCE_THRESHOLD) {
            distStatusEl.textContent = "Orang terdeteksi masuk!";
            distStatusEl.classList.add("warning");
        } else {
            distStatusEl.textContent = "Tidak ada orang terdeteksi";
            distStatusEl.classList.remove("warning");
        }
    }

    // 4. Update Cahaya (LDR)
    if (data.ldr !== undefined) {
        ldrEl.textContent = `Intensitas Cahaya: ${data.ldr}`;
        if (data.ldr < LDR_THRESHOLD) {
            ldrStatusEl.textContent = "Ruangan gelap, lampu menyala";
            ldrStatusEl.classList.add("warning"); // Menggunakan class warning untuk visual
        } else {
            ldrStatusEl.textContent = "Cahaya cukup";
            ldrStatusEl.classList.remove("warning");
        }
    }
}