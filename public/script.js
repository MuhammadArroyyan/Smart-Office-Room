// === Threshold (Harus sesuai dengan di ESP32) ===
const DISTANCE_THRESHOLD = 50; 
const HUMIDITY_THRESHOLD = 70; // Kelembapan dalam %
const LDR_THRESHOLD = 1000;

// === Elemen UI ===
const lastUpdatedEl = document.getElementById("last-updated");

// Blok kartu
const tempBlock = document.getElementById("temp-block");
const humBlock = document.getElementById("hum-block");
const distBlock = document.getElementById("dist-block");
const ldrBlock = document.getElementById("ldr-block");

// Elemen Data & Status
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
const sensorRef = ref(db, 'sensor/');
onValue(sensorRef, (snapshot) => {
    const data = snapshot.val();
    if (data) {
        updateUI(data);
        feather.replace(); 
    }
});

function updateUI(data) {
    // Update timestamp
    const now = new Date();
    lastUpdatedEl.textContent = `Terakhir Diperbarui: ${now.toLocaleTimeString('id-ID')}`;

    // 1. Update Suhu
    if (data.temperature !== undefined) {
        tempEl.textContent = data.temperature.toFixed(1);
        // Kalkulasi gauge (0-50°C range, 0-180deg)
        const tempRotation = (data.temperature / 50) * 180;
        tempGaugeEl.style.setProperty('--gauge-value', `${Math.min(tempRotation, 180)}deg`);
        
        tempStatusEl.textContent = "Status normal";
        tempStatusEl.classList.remove("warning");
        tempBlock.classList.remove("warning");
    }

    // 2. Update Kelembapan
    if (data.humidity !== undefined) {
        humEl.textContent = data.humidity.toFixed(0);
        // Kalkulasi gauge (0-100% range, 0-180deg)
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