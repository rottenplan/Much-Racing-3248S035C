"use client";

import { useEffect, useState } from "react";
import { MapContainer, TileLayer, Marker, Popup, useMap } from "react-leaflet";
import "leaflet/dist/leaflet.css";
import L from "leaflet";

// Fix Leaflet Default Icon in Next.js
// @ts-ignore
delete L.Icon.Default.prototype._getIconUrl;
L.Icon.Default.mergeOptions({
    iconRetinaUrl: "https://unpkg.com/leaflet@1.7.1/dist/images/marker-icon-2x.png",
    iconUrl: "https://unpkg.com/leaflet@1.7.1/dist/images/marker-icon.png",
    shadowUrl: "https://unpkg.com/leaflet@1.7.1/dist/images/marker-shadow.png",
});

function MapController({ lat, lng, follow }: { lat: number; lng: number, follow: boolean }) {
    const map = useMap();
    useEffect(() => {
        if (lat !== 0 && lng !== 0 && follow) {
            map.setView([lat, lng], map.getZoom());
        }
    }, [lat, lng, follow, map]);
    return null;
}

export default function SatelliteMap() {
    const [data, setData] = useState({ lat: 0, lng: 0, speed: 0, rpm: 0, trip: 0, sats: 0 });
    const [follow, setFollow] = useState(true);
    const [ip, setIp] = useState("192.168.4.1");
    const [connected, setConnected] = useState(false);
    const [isOnline, setIsOnline] = useState(true);

    useEffect(() => {
        setIsOnline(navigator.onLine);
        const handleOnline = () => setIsOnline(true);
        const handleOffline = () => setIsOnline(false);

        window.addEventListener('online', handleOnline);
        window.addEventListener('offline', handleOffline);

        return () => {
            window.removeEventListener('online', handleOnline);
            window.removeEventListener('offline', handleOffline);
        };
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

    return (
        <div className="w-full h-full flex flex-col">
            {/* Control Bar */}
            <div className="bg-gray-900 p-2 flex justify-between items-center text-xs border-b border-gray-700">
                <div className="flex gap-2 items-center">
                    <span className={`w-2 h-2 rounded-full ${connected ? 'bg-green-500' : 'bg-red-500'}`}></span>
                    <input
                        type="text"
                        value={ip}
                        onChange={(e) => setIp(e.target.value)}
                        className="bg-gray-800 text-white px-2 py-1 rounded border border-gray-700 w-28"
                    />
                </div>
                <button
                    onClick={() => setFollow(!follow)}
                    className={`px-3 py-1 rounded ${follow ? 'bg-blue-600' : 'bg-gray-700'}`}
                >
                    {follow ? 'Locked' : 'Free'}
                </button>
            </div>

            {/* Map */}
            <div className="flex-grow relative">
                {/* Connection Warnings */}
                {!isOnline && (
                    <div className="absolute top-2 left-1/2 transform -translate-x-1/2 z-[9999] bg-red-600/90 text-white px-4 py-2 rounded-full text-xs font-bold shadow-lg backdrop-blur">
                        ⚠️ No Internet - Map Tiles Won't Load
                    </div>
                )}
                {!connected && isOnline && (
                    <div className="absolute top-12 left-1/2 transform -translate-x-1/2 z-[9999] bg-orange-600/90 text-white px-4 py-2 rounded-full text-xs font-bold shadow-lg backdrop-blur">
                        📡 Connecting to RaceBox ({ip})...
                    </div>
                )}

                <MapContainer center={[0, 0]} zoom={2} style={{ height: "100%", width: "100%" }}>
                    <TileLayer
                        attribution='Tiles &copy; Esri'
                        url="https://server.arcgisonline.com/ArcGIS/rest/services/World_Imagery/MapServer/tile/{z}/{y}/{x}"
                    />
                    {data.lat !== 0 && (
                        <Marker position={[data.lat, data.lng]}>
                            <Popup>
                                Speed: {data.speed} km/h<br />
                                RPM: {data.rpm}<br />
                                Sats: {data.sats}
                            </Popup>
                        </Marker>
                    )}
                    <MapController lat={data.lat} lng={data.lng} follow={follow} />
                </MapContainer>

                {/* Overlay Stats */}
                <div className="absolute bottom-4 left-4 right-4 bg-black/80 backdrop-blur border border-green-500/50 p-4 rounded-xl grid grid-cols-2 gap-4 z-[9999]">
                    <div className="text-center">
                        <div className="text-gray-400 text-xs uppercase">Speed</div>
                        <div className="text-2xl font-bold text-white">{Math.round(data.speed)} <span className="text-sm text-green-500">KM/H</span></div>
                    </div>
                    <div className="text-center">
                        <div className="text-gray-400 text-xs uppercase">RPM</div>
                        <div className="text-2xl font-bold text-white">{data.rpm} <span className="text-sm text-green-500">RPM</span></div>
                    </div>
                    <div className="text-center">
                        <div className="text-gray-400 text-xs uppercase">Trip</div>
                        <div className="text-xl font-bold text-white">{data.trip.toFixed(0)} <span className="text-sm text-green-500">KM</span></div>
                    </div>
                    <div className="text-center">
                        <div className="text-gray-400 text-xs uppercase">Sats</div>
                        <div className="text-xl font-bold text-white">{data.sats}</div>
                    </div>
                </div>
            </div>
        </div>
    );
}
