import sys
import os
import socket

sys.path.append(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))

from pyftpdlib.servers import FTPServer
from pyftpdlib.handlers import FTPHandler
from config.config import Config
from utils.logger import FTPLogger
from server.auth import FTPAuth

class CustomFTPHandler(FTPHandler):
    def on_connect(self):
        self.server.logger.info(f"Cliente conectado desde {self.remote_ip}:{self.remote_port}")
    
    def on_disconnect(self):
        self.server.logger.info(f"Cliente desconectado: {self.remote_ip}:{self.remote_port}")
    
    def on_login(self, username):
        self.server.logger.info(f"Usuario autenticado: {username} desde {self.remote_ip}")
    
    def on_logout(self, username):
        self.server.logger.info(f"Usuario desconectado: {username}")
    
    def on_file_sent(self, file):
        self.server.logger.info(f"Archivo descargado: {file} por {self.username}")
    
    def on_file_received(self, file):
        self.server.logger.info(f"Archivo subido: {file} por {self.username}")

class FTPServerManager:
    def __init__(self, config_path='config.json'):
        self.logger = FTPLogger()
        self.config = Config(config_path)
        self.auth = FTPAuth(self.config, self.logger)
        self.server = None
    
    def _get_local_ip(self):
        try:
            s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
            s.connect(("8.8.8.8", 80))
            ip = s.getsockname()[0]
            s.close()
            return ip
        except:
            return "No disponible"
    
    def start(self):
        handler = CustomFTPHandler
        handler.authorizer = self.auth.get_authorizer()
        
        server_config = self.config.get_server_config()
        host = self.config.get_host()
        port = self.config.get_port()
        
        handler.passive_ports = range(60000, 60100)
        
        self.server = FTPServer((host, port), handler)
        self.server.logger = self.logger
        
        max_cons = server_config.get('max_connections', 10)
        max_cons_per_ip = server_config.get('max_connections_per_ip', 5)
        self.server.max_cons = max_cons
        self.server.max_cons_per_ip = max_cons_per_ip
        
        local_ip = self._get_local_ip()
        
        print("\n" + "="*70)
        print("SERVIDOR FTP INICIADO")
        print("="*70)
        print(f"Protocolo: FTP (Capa 7 - Aplicacion) sobre TCP")
        print(f"Puerto de control: {port}")
        print(f"Puertos pasivos: 60000-60100")
        print(f"IP Local: {local_ip}")
        print(f"Directorio raiz: {os.path.abspath(self.config.get_ftp_root())}")
        print(f"Conexiones maximas: {max_cons}")
        print(f"Conexiones por IP: {max_cons_per_ip}")
        print("\nUsuarios configurados:")
        for user in self.config.get_users():
            print(f"  - {user['username']} (permisos: {user['permissions']})")
        print("\nPara conectarse desde otra computadora:")
        print(f"  ftp {local_ip} {port}")
        print("\nPresione Ctrl+C para detener el servidor")
        print("="*70 + "\n")
        
        self.logger.info(f"Servidor FTP iniciado en {host}:{port}")
        
        try:
            self.server.serve_forever()
        except KeyboardInterrupt:
            self.logger.info("Servidor detenido por el usuario")
            print("\n\nServidor FTP detenido.")
        except Exception as e:
            self.logger.error(f"Error en el servidor: {e}")
            raise

def main():
    server_manager = FTPServerManager()
    server_manager.start()

if __name__ == '__main__':
    main()
