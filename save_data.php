<?php
$host = "localhost"; // Server MySQL (localhost jika di komputer lokal)
$user = "root";      // Username MySQL
$pass = "";          // Password MySQL (kosong jika default XAMPP)
$db = "iot_project"; // Nama database

// Koneksi ke database
$conn = new mysqli($host, $user, $pass, $db);

// Periksa koneksi
if ($conn->connect_error) {
    die("Koneksi gagal: " . $conn->connect_error);
}

// Ambil data dari ESP32
$rfid_id = $_POST['rfid_id'];
$berat = $_POST['berat'];
$tinggi = $_POST['tinggi'];

// Query untuk menyimpan data
$sql = "INSERT INTO users (rfid_id, berat, tinggi) VALUES ('$rfid_id', '$berat', '$tinggi')";

if ($conn->query($sql) === TRUE) {
    echo "Data berhasil disimpan";
} else {
    echo "Error: " . $sql . "<br>" . $conn->error;
}

$conn->close();
?>
