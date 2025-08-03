#!/usr/bin/php
<?php
// Required for CGI scripts to work correctly
header("Content-Type: text/html\r\n\r\n");

// Start the session before any output
session_start();

// Hardcoded credentials
$valid_username = 'admin';
$valid_password = '1234';

// Handle logout
if (isset($_GET['action']) && $_GET['action'] === 'logout') {
    // Clear session data
    $_SESSION = array();
    session_destroy();

    // Clear the login cookie
    setcookie("user", "", time() - 3600, "/");

    // Redirect to login page after logout
    header("Location: " . strtok($_SERVER["REQUEST_URI"], '?'));
    exit();
}

// Handle login submission
if ($_SERVER['REQUEST_METHOD'] === 'POST') {
    $username = isset($_POST['username']) ? $_POST['username'] : '';
    $password = isset($_POST['password']) ? $_POST['password'] : '';

    // Check credentials
    if ($username === $valid_username && $password === $valid_password) {
        // Set session variable
        $_SESSION['logged_in'] = true;

        // Set a cookie valid for 1 hour
        setcookie("user", $username, time() + 3600, "/");

        // Redirect to avoid resubmission on refresh
        header("Location: " . $_SERVER["REQUEST_URI"]);
        exit();
    } else {
        $error = "Invalid credentials.";
    }
}

// Check if user is already logged in via session
$logged_in = isset($_SESSION['logged_in']) && $_SESSION['logged_in'] === true;

// Begin HTML output
echo "<!DOCTYPE html>";
echo "<html><head><title>PHP CGI Login</title></head><body>";

if ($logged_in) {
    // If logged in, show welcome message and logout option
    $user = isset($_COOKIE['user']) ? htmlspecialchars($_COOKIE['user']) : 'Guest';
    echo "<h2>Welcome, $user!</h2>";
    echo "<p><a href='?action=logout'>Logout</a></p>";
} else {
    // If not logged in, show login form
    if (isset($error)) {
        echo "<p style='color:red;'>$error</p>";
    }
    echo <<<HTML
    <h2>Login</h2>
    <form method="post">
        <label>Username: <input type="text" name="username" /></label><br/>
        <label>Password: <input type="password" name="password" /></label><br/>
        <input type="submit" value="Login" />
    </form>
HTML;
}

echo "</body></html>";
?>
