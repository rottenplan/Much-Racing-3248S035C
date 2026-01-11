'use client';

import dynamic from "next/dynamic";

const TrackCreatorMap = dynamic(() => import("./TrackCreatorMap"), {
    ssr: false,
    loading: () => <div className="w-full h-full bg-slate-800 flex items-center justify-center text-slate-400">Loading Map...</div>
});

interface TrackCreatorMapProps {
    points: { lat: number; lng: number }[];
    onPointAdd: (lat: number, lng: number) => void;
}

export default function TrackCreatorMapWrapper(props: TrackCreatorMapProps) {
    return <TrackCreatorMap {...props} />;
}
