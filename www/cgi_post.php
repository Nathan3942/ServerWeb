#!/usr/bin/php-cgi
<?php
// Obligatoire pour CGI
header("Content-Type: text/html");

?>
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <title>CGI POST Test</title>
    <style>
        body {
            font-family: "Segoe UI", sans-serif;
            background-color: #1e1e2f;
            color: #f1f1f1;
            padding: 2rem;
        }
        h1 {
            color: #61dafb;
        }
        pre {
            background-color: #2e2e3e;
            padding: 1rem;
            border-radius: 8px;
            overflow-x: auto;
        }
    </style>
</head>
<body>
    <h1>🚀 CGI POST Test PHP</h1>

    <h2>🌐 $_POST</h2>
    <pre><?php print_r($_POST); ?></pre>

    <h2>🔒 $_SERVER</h2>
    <pre><?php
        $keys = ['REQUEST_METHOD', 'SCRIPT_FILENAME', 'SCRIPT_NAME', 'QUERY_STRING', 'CONTENT_TYPE', 'CONTENT_LENGTH'];
        foreach ($keys as $key) {
            echo "$key: " . ($_SERVER[$key] ?? 'N/A') . "\n";
        }
    ?></pre>

    <h2>🧠 Raw Input (php://input)</h2>
    <pre><?php echo file_get_contents("php://input"); ?></pre>
</body>
</html>
