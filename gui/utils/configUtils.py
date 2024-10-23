import os
import json

def loadConfig() -> dict:
    """
    加载配置，如果文件不存在则创建默认配置
    """
    config_path = os.path.join(os.path.dirname(__file__), '..', '..', 'config', 'config.json')

    # 检查配置文件是否存在
    if not os.path.exists(config_path):
        # 如果文件不存在，创建默认配置
        defaultConfig = {
            'settings': {
                'username': 'default_user',
                'password': 'default_password',
                'timeout': 30,
                'items': ['item1', 'item2', 'item3']
            }
        }
        writeConfig(defaultConfig)
    else:
        with open(config_path, 'r') as configfile:
            return json.load(configfile)

    return defaultConfig

def writeConfig(config: dict) -> None:
    """
    写入配置
    """
    config_path = os.path.join(os.path.dirname(__file__), '..', '..', 'config', 'config.json')

    with open(config_path, 'w') as configfile:
        json.dump(config, configfile, indent=4)

# 示例使用
if __name__ == "__main__":
    config = loadConfig()

    # 修改配置
    config['settings']['username'] = 'new_username'
    config['settings']['items'].append('item4')  # 添加新项目

    # 写回配置文件
    writeConfig(config)
