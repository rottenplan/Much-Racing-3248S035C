"use client";

import dynamic from "next/dynamic";

const SessionMap = dynamic(() => import("./SessionMap"), {
    ssr: false,
    loading: () => <div className="w-full h-full bg-slate-800 flex items-center justify-center text-slate-400">Loading Map...</div>
});

export default function SessionMapWrapper({ points }: { points: { lat: number; lng: number }[] }) {
    return <SessionMap points={points} />;
}
