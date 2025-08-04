#!/usr/bin/php-cgi
<?php
session_start();

header("Content-Type: text/html");

echo "<html><body>";
if (isset($_SESSION['username'])) {
    echo "<h1>Welcome, " . htmlspecialchars($_SESSION['username']) . "!</h1>";
    if (isset($_COOKIE['last_login'])) {
        echo "<p>Last login via cookie: " . htmlspecialchars($_COOKIE['last_login']) . "</p>";
    }
    echo "<a href='/cgi-bin/logout.php'>Logout</a>";
} else {
    echo "<p>You are not logged in.</p>";
    echo "<a href='/cgi-bin/login.php'>Go to login</a>";
}
echo "</body></html>";
