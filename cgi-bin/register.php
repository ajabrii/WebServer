#!/usr/bin/php-cgi
<?php
header("Content-Type: text/html");

if (!empty($_SERVER['QUERY_STRING'])) {
    // Fill $_GET manually in some CGI environments (optional)
    parse_str($_SERVER['QUERY_STRING'], $_GET);
}

$username = $_GET['username'] ?? '';
$password = $_GET['password'] ?? '';

if (!$username || !$password) {
    echo "<p>Missing username or password.</p>";
    echo "<a href='login.html'>Back</a>";
    exit;
}

$db = new SQLite3(__DIR__ . '/users.db');
$stmt = $db->prepare('INSERT INTO users (username, password) VALUES (:u, :p)');
$stmt->bindValue(':u', $username, SQLITE3_TEXT);
$stmt->bindValue(':p', $password, SQLITE3_TEXT);

try {
    $stmt->execute();
    session_start();
    $_SESSION['username'] = $username;
    header("Location: welcome.php");
    exit;
} catch (Exception $e) {
    echo "<p>Username already taken.</p>";
    echo "<a href='login.html'>Try again</a>";
}
?>
