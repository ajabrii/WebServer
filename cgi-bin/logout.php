#!/usr/bin/php-cgi
<?php
session_start();

// Destroy all session data
$_SESSION = [];
session_destroy();

// Delete the session cookie by setting expiration in the past
if (ini_get("session.use_cookies")) {
    $params = session_get_cookie_params();
    setcookie(session_name(), '', time() - 42000,
        $params["path"], $params["domain"],
        $params["secure"], $params["httponly"]
    );
}

// Delete your custom cookie (if any)
setcookie("last_login", "", time() - 3600, "/");

header("Location: /login.html"); // Redirect back to your login page
exit;
