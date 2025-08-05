#!/usr/bin/php-cgi
<?php
header("Content-Type: text/html");
$db = new SQLite3(__DIR__ . '/users.db');
$db->exec('CREATE TABLE IF NOT EXISTS users (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    username TEXT UNIQUE NOT NULL,
    password TEXT NOT NULL
)');
echo "<p>Database initialized.</p>";
?>
