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

import { useLiveTelemetry } from "../hooks/useLiveTelemetry";

// ... constants ...

export default function SatelliteMap() {
    const { data, ip, setIp, connected, isOnline } = useLiveTelemetry();
    const [follow, setFollow] = useState(true);

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
                    <div className="absolute top-2 left-1/2 transform -translate-x-1/2 z-[1000] bg-red-600/90 text-white px-4 py-2 rounded-full text-xs font-bold shadow-lg backdrop-blur">
                        ⚠️ No Internet - Map Tiles Won't Load
                    </div>
                )}
                {!connected && isOnline && (
                    <div className="absolute top-12 left-1/2 transform -translate-x-1/2 z-[1000] bg-orange-600/90 text-white px-4 py-2 rounded-full text-xs font-bold shadow-lg backdrop-blur">
                        📡 Connecting to Device ({ip})...
                    </div>
                )}

                <MapContainer center={[-6.2088, 106.8456]} zoom={13} style={{ height: "100%", width: "100%" }}>
                    {/* Google Maps Satellite (Hybrid) */}
                    <TileLayer
                        attribution='&copy; Google Maps'
                        url="https://mt0.google.com/vt/lyrs=y&hl=en&x={x}&y={y}&z={z}"
                        maxZoom={20}
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
                <div className="absolute bottom-2 left-2 right-2 bg-black/80 backdrop-blur border border-green-500/50 p-2 rounded-lg grid grid-cols-4 gap-2 z-[1000]">
                    <div className="text-center">
                        <div className="text-gray-400 text-[10px] uppercase">Speed</div>
                        <div className="text-lg font-bold text-white leading-tight">{Math.round(data.speed)} <span className="text-[10px] text-green-500">KMH</span></div>
                    </div>
                    <div className="text-center">
                        <div className="text-gray-400 text-[10px] uppercase">RPM</div>
                        <div className="text-lg font-bold text-white leading-tight">{data.rpm}</div>
                    </div>
                    <div className="text-center">
                        <div className="text-gray-400 text-[10px] uppercase">Trip</div>
                        <div className="text-lg font-bold text-white leading-tight">{data.trip.toFixed(0)} <span className="text-[10px] text-green-500">KM</span></div>
                    </div>
                    <div className="text-center">
                        <div className="text-gray-400 text-[10px] uppercase">Sats</div>
                        <div className="text-lg font-bold text-white leading-tight">{data.sats}</div>
                    </div>
                </div>
            </div>
        </div>
    );
}
