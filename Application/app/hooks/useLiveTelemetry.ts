import { useState, useEffect } from 'react';

export interface TelemetryData {
    lat: number;
    lng: number;
    speed: number;
    rpm: number;
    trip: number;
    sats: number;
}

export function useLiveTelemetry(initialIp: string = "192.168.4.1") {
    const [data, setData] = useState<TelemetryData>({ lat: 0, lng: 0, speed: 0, rpm: 0, trip: 0, sats: 0 });
    const [ip, setIp] = useState(initialIp);
    const [connected, setConnected] = useState(false);
    const [isOnline, setIsOnline] = useState(true);

    useEffect(() => {
        if (typeof window !== 'undefined') {
            setIsOnline(navigator.onLine);
            const handleOnline = () => setIsOnline(true);
            const handleOffline = () => setIsOnline(false);

            window.addEventListener('online', handleOnline);
            window.addEventListener('offline', handleOffline);

            return () => {
                window.removeEventListener('online', handleOnline);
                window.removeEventListener('offline', handleOffline);
            };
        }
    }, []);

    useEffect(() => {
        const interval = setInterval(() => {
            fetch(`http://${ip}/api/live`)
                .then((res) => res.json())
                .then((json) => {
                    setData(json);
                    setConnected(true);
                })
                .catch(() => setConnected(false));
        }, 1000);
        return () => clearInterval(interval);
    }, [ip]);

    return { data, ip, setIp, connected, isOnline };
}
