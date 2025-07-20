
<?php
$name = $_GET['name'] ?? 'Guest';
header('Content-Type: text/html');
echo "<h1>Hello, $name!</h1>";
?>
