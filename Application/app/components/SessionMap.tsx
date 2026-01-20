"use client";

import { useEffect, useState } from "react";
import { MapContainer, TileLayer, Polyline, Marker, Popup } from "react-leaflet";
import "leaflet/dist/leaflet.css";
import L from "leaflet";
import { MapPin } from "lucide-react";

// Fix Leaflet Icons
// @ts-ignore
delete L.Icon.Default.prototype._getIconUrl;
L.Icon.Default.mergeOptions({
    iconRetinaUrl: "https://unpkg.com/leaflet@1.7.1/dist/images/marker-icon-2x.png",
    iconUrl: "https://unpkg.com/leaflet@1.7.1/dist/images/marker-icon.png",
    shadowUrl: "https://unpkg.com/leaflet@1.7.1/dist/images/marker-shadow.png",
});

interface SessionMapProps {
    points: { lat: number; lng: number }[];
    startPoint?: { lat: number; lng: number };
}

// Sentul Circuit Approx Coords (Placeholder default)
const SENTUL_COORDS: [number, number] = [-6.535, 106.858];

export default function SessionMap({ points, startPoint }: SessionMapProps) {
    const [center, setCenter] = useState<[number, number]>(SENTUL_COORDS);

    useEffect(() => {
        if (points && points.length > 0) {
            // Calculate center of the bounds
            const lats = points.map(p => p.lat);
            const lngs = points.map(p => p.lng);
            const minLat = Math.min(...lats);
            const maxLat = Math.max(...lats);
            const minLng = Math.min(...lngs);
            const maxLng = Math.max(...lngs);

            setCenter([(minLat + maxLat) / 2, (minLng + maxLng) / 2]);
        }
    }, [points]);

    const polylinePositions = points.map(p => [p.lat, p.lng] as [number, number]);

    return (
        <div className="w-full h-full rounded-xl overflow-hidden relative z-0">
            <MapContainer center={center} zoom={16} style={{ height: "100%", width: "100%" }}>
                {/* Google Satellite */}
                <TileLayer
                    attribution='&copy; Google Maps'
                    url="https://mt0.google.com/vt/lyrs=y&hl=en&x={x}&y={y}&z={z}"
                    maxZoom={20}
                />

                {/* Racing Line */}
                <Polyline
                    positions={polylinePositions}
                    pathOptions={{ color: '#ef4444', weight: 4, opacity: 0.8 }}
                />

                {/* Start/Finish Marker */}
                {points.length > 0 && (
                    <Marker position={[points[0].lat, points[0].lng]}>
                        <Popup>Start/Finish</Popup>
                    </Marker>
                )}
            </MapContainer>

            {/* Map Overlay Label */}
            <div className="absolute top-4 left-4 z-[400] bg-black/50 backdrop-blur px-3 py-1 rounded-full text-xs font-racing border border-white/10 flex items-center gap-2">
                <MapPin className="w-3 h-3 text-primary" />
                SESSION TRAJECTORY
            </div>
        </div>
    );
}
