#!/usr/bin/php-cgi
<?php
header("Content-Type: text/html");
session_start();
session_destroy();
header("Location: ../login.html");
exit;
?>
