"use client";

import { useEffect } from "react";
import { MapContainer, TileLayer, Polyline, Marker, Popup, useMap } from "react-leaflet";
import "leaflet/dist/leaflet.css";
import L from "leaflet";

// Fix Leaflet Default Icon
// @ts-ignore
delete L.Icon.Default.prototype._getIconUrl;
L.Icon.Default.mergeOptions({
    iconRetinaUrl: "https://unpkg.com/leaflet@1.7.1/dist/images/marker-icon-2x.png",
    iconUrl: "https://unpkg.com/leaflet@1.7.1/dist/images/marker-icon.png",
    shadowUrl: "https://unpkg.com/leaflet@1.7.1/dist/images/marker-shadow.png",
});

function MapBounds({ points }: { points: [number, number][] }) {
    const map = useMap();
    useEffect(() => {
        if (points.length > 0) {
            const bounds = L.latLngBounds(points);
            map.fitBounds(bounds, { padding: [50, 50] });
        }
    }, [points, map]);
    return null;
}

export default function SessionMap({ points }: { points: { lat: number; lng: number }[] }) {
    const polylinePoints = points.map(p => [p.lat, p.lng] as [number, number]);
    const startPoint = polylinePoints[0];
    const endPoint = polylinePoints[polylinePoints.length - 1];

    // Calculate center
    const center = startPoint || [0, 0];

    return (
        <MapContainer center={center} zoom={13} style={{ height: "100%", width: "100%" }}>
            <TileLayer
                attribution='Tiles &copy; Esri'
                url="https://server.arcgisonline.com/ArcGIS/rest/services/World_Imagery/MapServer/tile/{z}/{y}/{x}"
            />
            {polylinePoints.length > 0 && (
                <>
                    <Polyline positions={polylinePoints} color="orange" weight={4} />
                    <Marker position={startPoint}>
                        <Popup>Start</Popup>
                    </Marker>
                    <Marker position={endPoint}>
                        <Popup>Finish</Popup>
                    </Marker>
                    <MapBounds points={polylinePoints} />
                </>
            )}
        </MapContainer>
    );
}
