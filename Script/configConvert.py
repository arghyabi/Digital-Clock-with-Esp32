# read the config.yaml file and convert it to config.h
import yaml
import os

def convertConfig(yamlFile, headerFile):
    with open(yamlFile, 'r') as f:
        config = yaml.safe_load(f)

    os.makedirs(os.path.dirname(headerFile), exist_ok=True)

    with open(headerFile, 'w') as f:
        f.write('#ifndef CLOCK_CONFIG_H\n')
        f.write('#define CLOCK_CONFIG_H\n\n')

        if 'appVersion' in config:
            f.write(f'#define APP_VERSION "{config["appVersion"]}"\n')

        if 'timeZone' in config:
            f.write(f'#define TIME_ZONE "{config["timeZone"]}"\n')

        if 'brightness' in config:
            f.write(f'#define DISPLAY_BRIGHTNESS {config["brightness"]}\n')

        if 'blinkInterval' in config:
            f.write(f'#define BLINK_INTERVAL {config["blinkInterval"]}\n')

        f.write('\n#endif // CLOCK_CONFIG_H\n')


if __name__ == "__main__":
    convertConfig('config.yaml', 'Src/AutoGen/config.h')
