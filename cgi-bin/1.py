#!/usr/bin/env python3


# Output required CGI header and blank line (MUST be CRLF CRLF)
print("Content-Type: text/html")

print("\r\n\r\n")

html = """
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <title>History of Morocco kindom</title>
    <style>
        body {
            font-family: 'Segoe UI', sans-serif;
            background-color: #f4f4f4;
            margin: 0;
            padding: 0;
            color: #333;
        }

        header {
            background-color: #2c3e50;
            color: white;
            padding: 30px 0;
            text-align: center;
        }

        .container {
            max-width: 800px;
            margin: 30px auto;
            padding: 20px;
            background-color: white;
            box-shadow: 0px 0px 10px rgba(0,0,0,0.1);
            border-radius: 10px;
        }

        h1, h2 {
            color: #2c3e50;
        }

        p {
            line-height: 1.6;
        }

        footer {
            text-align: center;
            font-size: 0.9em;
            color: #999;
            margin-top: 40px;
            padding-bottom: 20px;
        }
    </style>
</head>
<body>
    <header>
        <h1>History of Kingdom of Morocco</h1>
    </header>
    <div class="container">
        <h2>Ancient Morocco</h2>
        <p>Morocco has been inhabited since the Paleolithic period, over 300,000 years ago. The earliest known Moroccan civilization was the indigenous Berber culture. Carthaginians and Romans also left their mark, particularly in cities like Volubilis.</p>

        <h2>The Islamic and Almoravid Period</h2>
        <p>Islam arrived in the 7th century with Arab conquests. The Almoravid dynasty was founded in the 11th century and established Marrakech as a political and religious center. This period marked the beginning of powerful Moroccan Islamic empires.</p>

        <h2>The Almohads and Marinids</h2>
        <p>The Almohads succeeded the Almoravids in the 12th century, extending control into Spain. Later, the Marinid dynasty flourished in the 13th–15th centuries, supporting arts, architecture, and education.</p>

        <h2>European Influence and Colonial Era</h2>
        <p>From the 15th century, Portuguese and Spanish forces began occupying parts of Morocco’s coastline. In 1912, Morocco became a French and Spanish protectorate, with heavy colonial influence lasting until independence in 1956.</p>

        <h2>Modern Morocco</h2>
        <p>After gaining independence, Morocco was ruled by King Mohammed V, followed by Hassan II and now Mohammed VI. The country has seen economic reforms, development, and increased political openness in the 21st century.</p>
    </div>
    <footer>
        &copy; 2025 | CGI Blog | Made with Python 🐍
    </footer>
</body>
</html>
"""

print(html)
