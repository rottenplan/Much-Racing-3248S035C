'use client';

import { MapContainer, TileLayer, Polyline, CircleMarker, useMapEvents, LayersControl } from 'react-leaflet';
import 'leaflet/dist/leaflet.css';
import L from 'leaflet';

// Fix for default markers
delete (L.Icon.Default.prototype as any)._getIconUrl;
L.Icon.Default.mergeOptions({
    iconRetinaUrl: 'https://cdnjs.cloudflare.com/ajax/libs/leaflet/1.7.1/images/marker-icon-2x.png',
    iconUrl: 'https://cdnjs.cloudflare.com/ajax/libs/leaflet/1.7.1/images/marker-icon.png',
    shadowUrl: 'https://cdnjs.cloudflare.com/ajax/libs/leaflet/1.7.1/images/marker-shadow.png',
});

function MapEvents({ onMapClick }: { onMapClick: (lat: number, lng: number) => void }) {
    useMapEvents({
        click(e) {
            onMapClick(e.latlng.lat, e.latlng.lng);
        },
    });
    return null;
}

interface TrackCreatorMapProps {
    points: { lat: number; lng: number }[];
    onPointAdd: (lat: number, lng: number) => void;
}

export default function TrackCreatorMap({ points, onPointAdd }: TrackCreatorMapProps) {
    const polylinePoints = points.map(p => [p.lat, p.lng] as [number, number]);

    // Default center (e.g. Sentul) or last point
    const center: [number, number] = points.length > 0
        ? [points[points.length - 1].lat, points[points.length - 1].lng]
        : [-6.5360, 106.8520];

    return (
        <div className="w-full h-full">
            <MapContainer
                center={center}
                zoom={16}
                className="w-full h-full"
                style={{ background: '#1e293b' }}
            >
                <LayersControl position="topright">
                    <LayersControl.BaseLayer checked name="Google Hybrid">
                        <TileLayer
                            url="http://mt0.google.com/vt/lyrs=y&hl=en&x={x}&y={y}&z={z}"
                            subdomains={['mt0', 'mt1', 'mt2', 'mt3']}
                            maxZoom={20}
                            attribution='&copy; Google Maps'
                        />
                    </LayersControl.BaseLayer>

                    <LayersControl.BaseLayer name="Google Satellite (Clean)">
                        <TileLayer
                            url="http://mt0.google.com/vt/lyrs=s&hl=en&x={x}&y={y}&z={z}"
                            subdomains={['mt0', 'mt1', 'mt2', 'mt3']}
                            maxZoom={20}
                            attribution='&copy; Google Maps'
                        />
                    </LayersControl.BaseLayer>

                    <LayersControl.BaseLayer name="Bing/Esri Satellite">
                        <TileLayer
                            url="https://server.arcgisonline.com/ArcGIS/rest/services/World_Imagery/MapServer/tile/{z}/{y}/{x}"
                            maxZoom={19}
                            attribution='&copy; Esri'
                        />
                    </LayersControl.BaseLayer>

                    <LayersControl.BaseLayer name="OpenStreetMap">
                        <TileLayer
                            url="https://{s}.tile.openstreetmap.org/{z}/{x}/{y}.png"
                            maxZoom={19}
                            attribution='&copy; OpenStreetMap contributors'
                        />
                    </LayersControl.BaseLayer>
                </LayersControl>

                <MapEvents onMapClick={onPointAdd} />

                {points.length > 0 && (
                    <>
                        <Polyline
                            positions={polylinePoints}
                            color="#f97316" // Orange
                            weight={4}
                        />
                        {points.map((p, idx) => (
                            <CircleMarker
                                key={idx}
                                center={[p.lat, p.lng]}
                                radius={6}
                                pathOptions={{
                                    fillColor: idx === 0 ? '#22c55e' : '#f97316', // Green for start, Orange for others
                                    fillOpacity: 1,
                                    color: 'white',
                                    weight: 2
                                }}
                            />
                        ))}
                    </>
                )}
            </MapContainer>
        </div>
    );
}
