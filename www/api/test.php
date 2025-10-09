<?php
echo "Content-Type: text/html\r\n\r\n";

echo "<html><body>";
echo "<h1>Hello from PHP-CGI!</h1>";

echo "<h2>Request Info</h2>";
echo "<ul>";
echo "<li>Method: " . $_SERVER['REQUEST_METHOD'] . "</li>";
echo "<li>Query String: " . $_SERVER['QUERY_STRING'] . "</li>";
echo "<li>Script Name: " . $_SERVER['SCRIPT_NAME'] . "</li>";
echo "</ul>";

if ($_SERVER['REQUEST_METHOD'] === 'POST') {
    $postData = file_get_contents("php://input");
    echo "<h2>POST Body</h2><pre>" . htmlspecialchars($postData) . "</pre>";
}

echo "</body></html>";
?>