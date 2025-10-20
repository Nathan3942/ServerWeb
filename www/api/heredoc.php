#!/usr/bin/env php
<?php
// Nécessaire pour exécuter en CLI CGI
header("Content-Type: text/plain");

// Récupère la méthode HTTP
$method = $_SERVER['REQUEST_METHOD'] ?? 'GET';

if ($method === 'POST') {
    $length = (int)($_SERVER['CONTENT_LENGTH'] ?? 0);
    $body = file_get_contents("php://input", false, null, 0, $length);
    echo "POST reçu :\n";
    echo $body;
} else {
    echo "GET reçu\n";
}
?>
