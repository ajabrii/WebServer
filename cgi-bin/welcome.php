#!/usr/bin/php-cgi
<?php
header("Content-Type: text/html");
session_start();

if (!isset($_SESSION['username'])) {
    header("Location: ../login.html");
    exit;
}
?>
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Welcome Dashboard</title>
    <style>
        * {
            margin: 0;
            padding: 0;
            box-sizing: border-box;
        }

        body {
            font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            background-size: 400% 400%;
            animation: gradientShift 15s ease infinite;
            min-height: 100vh;
            display: flex;
            align-items: center;
            justify-content: center;
            padding: 20px;
        }

        @keyframes gradientShift {
            0% {
                background-position: 0% 50%;
            }
            50% {
                background-position: 100% 50%;
            }
            100% {
                background-position: 0% 50%;
            }
        }

        .dashboard-container {
            background: white;
            border-radius: 20px;
            box-shadow: 0 15px 35px rgba(0, 0, 0, 0.1);
            padding: 40px;
            width: 100%;
            max-width: 600px;
            backdrop-filter: blur(10px);
            animation: slideUp 0.8s ease-out;
            transform-origin: center bottom;
        }

        @keyframes slideUp {
            0% {
                opacity: 0;
                transform: translateY(50px) scale(0.9);
            }
            100% {
                opacity: 1;
                transform: translateY(0) scale(1);
            }
        }

        .header-section {
            text-align: center;
            margin-bottom: 40px;
            animation: fadeInDown 0.6s ease-out;
            animation-delay: 0.2s;
            animation-fill-mode: both;
        }

        @keyframes fadeInDown {
            0% {
                opacity: 0;
                transform: translateY(-20px);
            }
            100% {
                opacity: 1;
                transform: translateY(0);
            }
        }

        .welcome-icon {
            width: 80px;
            height: 80px;
            margin: 0 auto 20px;
            border-radius: 50%;
            background: linear-gradient(45deg, #667eea, #764ba2);
            display: flex;
            align-items: center;
            justify-content: center;
            font-size: 36px;
            color: white;
            animation: iconBounce 0.8s ease-out;
            animation-delay: 0.4s;
            animation-fill-mode: both;
            box-shadow: 0 8px 25px rgba(102, 126, 234, 0.3);
        }

        @keyframes iconBounce {
            0% {
                transform: scale(0);
                opacity: 0;
            }
            50% {
                transform: scale(1.2);
                opacity: 1;
            }
            100% {
                transform: scale(1);
                opacity: 1;
            }
        }

        h1 {
            color: #333;
            font-size: 32px;
            font-weight: 300;
            margin-bottom: 10px;
        }

        .username {
            background: linear-gradient(45deg, #667eea, #764ba2);
            -webkit-background-clip: text;
            -webkit-text-fill-color: transparent;
            background-clip: text;
            font-weight: 600;
        }

        .subtitle {
            color: #666;
            font-size: 16px;
            margin-bottom: 30px;
        }

        .dashboard-content {
            animation: fadeInUp 0.6s ease-out;
            animation-delay: 0.6s;
            animation-fill-mode: both;
        }

        @keyframes fadeInUp {
            0% {
                opacity: 0;
                transform: translateY(30px);
            }
            100% {
                opacity: 1;
                transform: translateY(0);
            }
        }

        .stats-grid {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(150px, 1fr));
            gap: 20px;
            margin-bottom: 40px;
        }

        .stat-card {
            background: linear-gradient(145deg, #f8f9fa, #e9ecef);
            border-radius: 15px;
            padding: 25px 20px;
            text-align: center;
            box-shadow: 0 4px 15px rgba(0, 0, 0, 0.1);
            transition: all 0.3s ease;
            animation: scaleIn 0.5s ease-out;
            animation-fill-mode: both;
        }

        .stat-card:nth-child(1) { animation-delay: 0.8s; }
        .stat-card:nth-child(2) { animation-delay: 0.9s; }
        .stat-card:nth-child(3) { animation-delay: 1.0s; }

        @keyframes scaleIn {
            0% {
                opacity: 0;
                transform: scale(0.8);
            }
            100% {
                opacity: 1;
                transform: scale(1);
            }
        }

        .stat-card:hover {
            transform: translateY(-5px);
            box-shadow: 0 8px 25px rgba(0, 0, 0, 0.15);
        }

        .stat-icon {
            font-size: 24px;
            margin-bottom: 10px;
        }

        .stat-value {
            font-size: 24px;
            font-weight: 600;
            color: #333;
            margin-bottom: 5px;
        }

        .stat-label {
            font-size: 14px;
            color: #666;
            text-transform: uppercase;
            letter-spacing: 1px;
        }

        .action-buttons {
            display: flex;
            gap: 15px;
            justify-content: center;
            flex-wrap: wrap;
            animation: slideInUp 0.6s ease-out;
            animation-delay: 1.2s;
            animation-fill-mode: both;
        }

        @keyframes slideInUp {
            0% {
                opacity: 0;
                transform: translateY(20px);
            }
            100% {
                opacity: 1;
                transform: translateY(0);
            }
        }

        .btn {
            padding: 12px 25px;
            border: none;
            border-radius: 10px;
            font-size: 16px;
            font-weight: 600;
            cursor: pointer;
            transition: all 0.3s ease;
            text-decoration: none;
            display: inline-flex;
            align-items: center;
            gap: 8px;
            text-transform: uppercase;
            letter-spacing: 1px;
        }

        .btn-primary {
            background: linear-gradient(45deg, #667eea, #764ba2);
            color: white;
            box-shadow: 0 4px 15px rgba(102, 126, 234, 0.2);
        }

        .btn-secondary {
            background: linear-gradient(45deg, #6c757d, #495057);
            color: white;
            box-shadow: 0 4px 15px rgba(108, 117, 125, 0.2);
        }

        .btn-danger {
            background: linear-gradient(45deg, #dc3545, #c82333);
            color: white;
            box-shadow: 0 4px 15px rgba(220, 53, 69, 0.2);
        }

        .btn:hover {
            transform: translateY(-2px);
            box-shadow: 0 8px 25px rgba(0, 0, 0, 0.2);
        }

        .btn:active {
            transform: translateY(0);
        }

        @media (max-width: 480px) {
            .dashboard-container {
                padding: 30px 20px;
                margin: 10px;
            }
            
            h1 {
                font-size: 28px;
            }
            
            .stats-grid {
                grid-template-columns: 1fr;
            }
            
            .action-buttons {
                flex-direction: column;
                align-items: center;
            }
            
            .btn {
                width: 100%;
                justify-content: center;
            }
        }
    </style>
</head>
<body>
    <div class="dashboard-container">
        <div class="header-section">
            <div class="welcome-icon">👋</div>
            <h1>Welcome back, <span class="username"><?= htmlspecialchars($_SESSION['username']) ?></span>!</h1>
            <p class="subtitle">You're successfully logged in to your dashboard</p>
        </div>
        
        <div class="dashboard-content">
            <div class="stats-grid">
                <div class="stat-card">
                    <div class="stat-icon">⏰</div>
                    <div class="stat-value" id="session-time">00:00</div>
                    <div class="stat-label">Session Time</div>
                </div>
                <div class="stat-card">
                    <div class="stat-icon">🛡️</div>
                    <div class="stat-value">Active</div>
                    <div class="stat-label">Status</div>
                </div>
                <div class="stat-card">
                    <div class="stat-icon">🌟</div>
                    <div class="stat-value">Premium</div>
                    <div class="stat-label">Account</div>
                </div>
            </div>
            
            <div class="action-buttons">
                <a href="../index.html" class="btn btn-primary">
                    🏠 Dashboard
                </a>
                <a href="../test-get.html" class="btn btn-secondary">
                    🔧 Test Suite
                </a>
                <a href="logout.php" class="btn btn-danger">
                    🚪 Logout
                </a>
            </div>
        </div>
    </div>

    <script>
        // Session timer
        let sessionStart = Date.now();
        
        function updateSessionTime() {
            const elapsed = Math.floor((Date.now() - sessionStart) / 1000);
            const minutes = Math.floor(elapsed / 60);
            const seconds = elapsed % 60;
            document.getElementById('session-time').textContent = 
                `${minutes.toString().padStart(2, '0')}:${seconds.toString().padStart(2, '0')}`;
        }
        
        setInterval(updateSessionTime, 1000);
        updateSessionTime();
    </script>
</body>
</html>
