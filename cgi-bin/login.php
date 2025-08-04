#!/usr/bin/php
<?php
// Enable output buffering to prevent premature header output
ob_start();

// Start or resume the session
session_start();

// Set hardcoded credentials
$valid_user = 'admin';
$valid_pass = '1234';

// Handle logout action
if (isset($_GET['action']) && $_GET['action'] === 'logout') {
    // Clear session variables
    $_SESSION = array();
    // Delete session cookie
    if (ini_get("session.use_cookies")) {
        $params = session_get_cookie_params();
        setcookie(session_name(), '', time() - 42000, $params["path"]);
    }
    // Destroy the session
    session_destroy();
    // Clear custom login cookie
    setcookie('login', '', time() - 3600, '/');
    // Redirect to login form
    header("Location: " . $_SERVER['SCRIPT_NAME']);
    exit;
}

// Check if already logged in via session or cookie
$loggedIn = false;
if (isset($_SESSION['user']) && $_SESSION['user'] === $valid_user) {
    $loggedIn = true;
} elseif (isset($_COOKIE['login']) && $_COOKIE['login'] === $valid_user) {
    // If cookie is present but no session, restore session
    $_SESSION['user'] = $_COOKIE['login'];
    $loggedIn = true;
}

// Handle login form submission
if (!$loggedIn && $_SERVER['REQUEST_METHOD'] === 'POST') {
    $user = $_POST['username'] ?? '';
    $pass = $_POST['password'] ?? '';
    if ($user === $valid_user && $pass === $valid_pass) {
        // Set session variable
        $_SESSION['user'] = $valid_user;
        // Set a cookie to persist login
        setcookie('login', $valid_user, time() + 3600, '/'); // 1 hour
        $loggedIn = true;
        // Redirect to avoid form resubmission
        header("Location: " . $_SERVER['SCRIPT_NAME']);
        exit;
    } else {
        $error = "Invalid credentials";
    }
}

// Output headers
header("Content-Type: text/html");

// Output body
?>
<!DOCTYPE html>
<html>
<head>
    <title>Login System</title>
</head>
<body>
<?php if ($loggedIn): ?>
    <h1>Welcome, <?= htmlspecialchars($valid_user) ?>!</h1>
    <p><a href="?action=logout">Logout</a></p>
<?php else: ?>
    <h1>Login</h1>
    <?php if (isset($error)): ?>
        <p style="color:red;"><?= htmlspecialchars($error) ?></p>
    <?php endif; ?>
    <form method="post" action="<?= htmlspecialchars($_SERVER['SCRIPT_NAME']) ?>">
        <label>Username: <input type="text" name="username" /></label><br />
        <label>Password: <input type="password" name="password" /></label><br />
        <input type="submit" value="Login" />
    </form>
<?php endif; ?>
</body>
</html>
<?php
// Flush the output buffer
ob_end_flush();
?>
