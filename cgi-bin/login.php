#!/usr/bin/php-cgi
<?php
header("Content-Type: text/html");
session_start();

if (isset($_SESSION['username'])) {
    // Already logged in, redirect to welcome page
    header("Location: welcome.php");
    exit;
}

if (!empty($_SERVER['QUERY_STRING'])) {
    parse_str($_SERVER['QUERY_STRING'], $_GET);
}

$username = $_GET['username'] ?? '';
$password = $_GET['password'] ?? '';

if (!$username || !$password) {
    echo "<p>Missing username or password.</p><a href='../login.html'>Back</a>";
    exit;
}

$file = __DIR__ . '/users.txt';

if (!file_exists($file)) {
    echo "<p>No users registered yet.</p><a href='../login.html'>Register first</a>";
    exit;
}

$users = file($file, FILE_IGNORE_NEW_LINES | FILE_SKIP_EMPTY_LINES);
$valid = false;
foreach ($users as $line) {
    list($u, $p) = explode(':', $line, 2);
    if ($u === $username && $p === $password) {
        $valid = true;
        break;
    }
}

if ($valid) {
    $_SESSION['username'] = $username;
    header("Location: welcome.php");
    exit;
} else {
    echo "<p>Invalid username or password.</p><a href='../login.html'>Try again</a>";
}
?>
