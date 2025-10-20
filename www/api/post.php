<!DOCTYPE html>
<html lang="fr">
<head>
    <meta charset="UTF-8">
    <title>Test CGI POST</title>
</head>
<body>
    <h1>Test du CGI en POST</h1>
    <form action="cgi_post.php" method="POST">
        <label>Nom : <input type="text" name="name"></label><br>
        <label>Message : <input type="text" name="message"></label><br>
        <input type="submit" value="Envoyer POST">
    </form>
</body>
</html>