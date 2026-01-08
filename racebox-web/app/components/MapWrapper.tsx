"use client";

import dynamic from "next/dynamic";

const SatelliteMap = dynamic(() => import("./SatelliteMap"), {
    ssr: false,
    loading: () => <div className="w-full h-full bg-gray-900 flex items-center justify-center text-white">Loading Map Engine...</div>
});

export default function MapWrapper() {
    return <SatelliteMap />;
}
