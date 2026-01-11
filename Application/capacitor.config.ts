import type { CapacitorConfig } from '@capacitor/cli';

const config: CapacitorConfig = {
  appId: 'com.muchracing.app',
  appName: 'RaceBox',
  webDir: 'out',
  server: {
    url: 'http://192.168.1.3:3000',
    cleartext: true
  }
};

export default config;
