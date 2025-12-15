import json
import os
import sys

class Config:
    def __init__(self, config_path='config.json'):
        self.config_path = config_path
        self.config = self._load_config()
    
    def _load_config(self):
        if not os.path.exists(self.config_path):
            print(f"Error: No se encontro el archivo de configuracion: {self.config_path}")
            sys.exit(1)
        
        try:
            with open(self.config_path, 'r', encoding='utf-8') as f:
                config = json.load(f)
            return config
        except json.JSONDecodeError as e:
            print(f"Error al leer configuracion: {e}")
            sys.exit(1)
    
    def get_server_config(self):
        return self.config.get('server', {})
    
    def get_directories(self):
        return self.config.get('directories', {})
    
    def get_users(self):
        return self.config.get('users', [])
    
    def get_ftp_root(self):
        return self.config.get('directories', {}).get('ftp_root', 'data/ftp_files')
    
    def get_host(self):
        return self.config.get('server', {}).get('host', '0.0.0.0')
    
    def get_port(self):
        return self.config.get('server', {}).get('port', 2121)
