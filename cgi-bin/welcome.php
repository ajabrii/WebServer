#!/usr/bin/php-cgi
<?php
header("Content-Type: text/html");
session_start();

if (!isset($_SESSION['username'])) {
    header("Location: ../login.html");
    exit;
}
?>
<!DOCTYPE html>
<html>
<head><title>Welcome</title></head>
<body>
  <h1>Welcome, <?= htmlspecialchars($_SESSION['username']) ?></h1>
  <a href="logout.php">Logout</a>
</body>
</html>
