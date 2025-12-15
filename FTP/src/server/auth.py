from pyftpdlib.authorizers import DummyAuthorizer
import sys
import os

sys.path.append(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
from utils.logger import FTPLogger

class FTPAuth:
    def __init__(self, config, logger):
        self.config = config
        self.logger = logger
        self.authorizer = DummyAuthorizer()
        self._setup_users()
    
    def _setup_users(self):
        ftp_root = os.path.abspath(self.config.get_ftp_root())
        os.makedirs(ftp_root, exist_ok=True)
        
        users = self.config.get_users()
        
        if not users:
            self.logger.warning("No hay usuarios configurados en config.json")
            return
        
        for user in users:
            username = user.get('username')
            password = user.get('password')
            permissions = user.get('permissions', 'elr')
            
            try:
                self.authorizer.add_user(username, password, ftp_root, perm=permissions)
                self.logger.info(f"Usuario agregado: {username} con permisos: {permissions}")
            except Exception as e:
                self.logger.error(f"Error al agregar usuario {username}: {e}")
    
    def get_authorizer(self):
        return self.authorizer
