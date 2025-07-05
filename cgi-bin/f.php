<?php
$name = $_GET['name'] ?? 'Guest';
echo "Content-Type: text/html\n\n";
echo "<h1>Hello, $name!</h1>";
?>
