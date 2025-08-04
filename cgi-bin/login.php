#!/usr/bin/php-cgi
<?php
session_start();

// Redirect if already logged in
if (isset($_SESSION['username'])) {
    header("Location: /cgi-bin/welcome.php");
    exit;
}

// Debug output (optional, only for development)
if (!empty($_SERVER['QUERY_STRING'])) {
    // Fill $_GET manually in some CGI environments (optional)
    parse_str($_SERVER['QUERY_STRING'], $_GET);
}

// Prepare headers early
header("Content-Type: text/html");

// Handle login via GET parameters
$username = $_GET['username'] ?? '';
$password = $_GET['password'] ?? '';

if ($username && $password) {
    if ($username === 'admin' && $password === '1234') {
        $_SESSION['username'] = $username;
        setcookie("last_login", $username, time() + 3600, "/");

        // Redirect after successful login
        header("Location: /cgi-bin/welcome.php");
        exit;
    } else {
        echo "<html><body><p style='color:red;'>Invalid credentials.</p></body></html>";
        exit;
    }
}
?>
<html>
<body>
    <h1>Login</h1>
    <form method="GET" action="/cgi-bin/login.php">
        <label>Username: <input type="text" name="username" required></label><br><br>
        <label>Password: <input type="text" name="password" required></label><br><br>
        <input type="submit" value="Login">
    </form>
</body>
</html>
