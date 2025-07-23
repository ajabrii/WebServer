import socket
import time

# ==== CONFIGURATION ====
SERVER_HOST = '127.0.0.3'
SERVER_PORT = 8081  # Bdel l port 3la 7sab l config dyalk
# =======================

def run_test(test_name, request_payload, expected_status_families):
    """
    Hadi fonction katkheddem test o katrjje3 result: PASS wlla FAIL.
    """
    try:
        client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        client_socket.settimeout(2.0)
        client_socket.connect((SERVER_HOST, SERVER_PORT))

        if isinstance(request_payload, list):
            for part in request_payload:
                client_socket.sendall(part)
                time.sleep(0.1)
        else:
            client_socket.sendall(request_payload)

        # Stqbel l jawab
        response = client_socket.recv(4096).decode('utf-8', errors='ignore')
        
        if not response:
            return {"name": test_name, "status": "FAIL", "reason": "L Server ma jawb b walo."}

        first_line = response.split('\r\n')[0]
        parts = first_line.split(' ')
        if len(parts) < 2 or not parts[1].isdigit():
             return {"name": test_name, "status": "FAIL", "reason": f"Response ghalta: '{first_line}'"}
        
        status_code_str = parts[1]
        
        for family in expected_status_families:
            if status_code_str.startswith(family):
                return {"name": test_name, "status": "PASS", "reason": f"L Server jawb b {status_code_str}"}
        
        return {"name": test_name, "status": "FAIL", "reason": f"Kantsennaw {expected_status_families}xx walakin lqina {status_code_str}"}

    except socket.timeout:
        return {"name": test_name, "status": "FAIL", "reason": "Timeout - L Server matjawbch f lweqt."}
    except Exception as e:
        return {"name": test_name, "status": "FAIL", "reason": f"Error: {e}"}
    finally:
        if 'client_socket' in locals():
            client_socket.close()

# ==============================================================================
# ====>>               LISTE DYAL LES TEST CASES                          <<====
# ==============================================================================

# 1. Tests li khasshom ydozo (Expected: 2xx wlla 3xx status)
valid_tests = [
    ("GET S7i7 l l'index", b"GET / HTTP/1.1\r\nHost: localhost\r\n\r\n"),
    ("POST l blassa d upload", b"POST /upload/test.txt HTTP/1.1\r\nHost: localhost\r\nContent-Length: 5\r\n\r\nhello"),
    ("DELETE l chi resource", b"DELETE /upload/test.txt HTTP/1.1\r\nHost: localhost\r\n\r\n"),
    ("GET l page fiha redirection", b"GET /old-page/ HTTP/1.1\r\nHost: localhost\r\n\r\n"),
]

# 2. Tests li khasshom yfailiw b error dyal client (Expected: 4xx status)
invalid_tests = [
    # -- Logic Ghalet --
    ("GET l fichier makayench", b"GET /makanench.html HTTP/1.1\r\nHost: localhost\r\n\r\n"),
    ("Method mamnou3a (POST)", b"POST /images/ HTTP/1.1\r\nHost: localhost\r\nContent-Length: 4\r\n\r\nbody"),
    
    # -- Parsing Ghalet: Request Line --
    ("Request Line: Bla Method", b"/ HTTP/1.1\r\nHost: localhost\r\n\r\n"),
    ("Request Line: Bzaf d les Espaces", b"GET    /    HTTP/1.1\r\nHost: localhost\r\n\r\n"),
    ("Request Line: Version ghalta", b"GET / HTTP/1.2\r\nHost: localhost\r\n\r\n"),
    
    # -- Parsing Ghalet: Headers --
    ("Headers: Bla Host", b"GET / HTTP/1.1\r\nUser-Agent: Me\r\n\r\n"),
    ("Headers: Host khawi", b"GET / HTTP/1.1\r\nHost: \r\n\r\n"),
    ("Headers: Bla colon", b"GET / HTTP/1.1\r\nHost localhost\r\n\r\n"),
    ("Headers: Espace qbel l colon", b"GET / HTTP/1.1\r\nHost : localhost\r\n\r\n"),
    
    # -- Parsing Ghalet: Body --
    ("Body: CL o TE bjouj", b"POST / HTTP/1.1\r\nHost: localhost\r\nContent-Length: 5\r\nTransfer-Encoding: chunked\r\n\r\n"),
    ("Body: CL machi raqem", b"POST / HTTP/1.1\r\nHost: localhost\r\nContent-Length: abc\r\n\r\nhello"),
    ("Body: Sgher men CL (Incomplete)", [b"POST / HTTP/1.1\r\nHost: localhost\r\nContent-Length: 20\r\n\r\nThisIsOnly12"]),
]

# ==============================================================================
# ====>>               Kheddem Les Tests o Jme3 l Results                 <<====
# ==============================================================================
if __name__ == "__main__":
    all_results = []
    
    print("Kanbda newsl l server dyalk...\n")

    # Kheddem les tests s7a7 (kansennaw 2xx wlla 3xx)
    for name, payload in valid_tests:
        all_results.append(run_test(name, payload, expected_status_families=['2', '3']))

    # Kheddem les tests ghalțin (kansennaw 4xx)
    for name, payload in invalid_tests:
        all_results.append(run_test(name, payload, expected_status_families=['4']))
        
    # ==============================================================================
    # ====>>                       L REPORT L NIHAEI                          <<====
    # ==============================================================================
    print("\n\n" + "="*60)
    print("====              R E P O R T   N I H A E I               ====")
    print("="*60)
    
    passed_count = 0
    failed_count = 0
    
    for result in all_results:
        if result["status"] == "PASS":
            print(f"  [  \033[92mPASS\033[0m  ] {result['name']:<45} | {result['reason']}")
            passed_count += 1
        else:
            print(f"  [  \033[91mFAIL\033[0m  ] {result['name']:<45} | {result['reason']}")
            failed_count += 1
            
    print("-" * 60)
    print(f"Total: {len(all_results)} | S7a7 (Passed): \033[92m{passed_count}\033[0m | Ghalțin (Failed): \033[91m{failed_count}\033[0m")
    print("="*60)
