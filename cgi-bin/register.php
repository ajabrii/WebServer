#!/usr/bin/php-cgi
<?php
header("Content-Type: text/html");

if (!empty($_SERVER['QUERY_STRING'])) {
    parse_str($_SERVER['QUERY_STRING'], $_GET);
}

$username = $_GET['username'] ?? '';
$password = $_GET['password'] ?? '';

if (!$username || !$password) 
{
    echo "<p>Missing username or password.</p><a href='../login.html'>Back</a>";
    exit;
}

$file = __DIR__ . '/users.txt';

// Check if user already exists
$users = file_exists($file) ? file($file, FILE_IGNORE_NEW_LINES | FILE_SKIP_EMPTY_LINES) : [];

foreach ($users as $line) 
{
    list($u,) = explode(':', $line, 2);
    if ($u === $username) 
    {
        echo "<p>Username already taken.</p><a href='../login.html'>Try again</a>";
        exit;
    }
}

// Append new user (plain password — NOT secure, only for demo/testing)
file_put_contents($file, "$username:$password\n", FILE_APPEND);

session_start();
$_SESSION['username'] = $username;
header("Location: welcome.php");
exit;
?>
