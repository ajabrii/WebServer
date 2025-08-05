
<?php
header('Content-Type: text/html');
$name = $_GET['name'] ?? 'Guest';
echo "<h1>Hello, $name</h1>";
?>
