"use client";

import {
    ChevronLeft,
    MoreHorizontal,
    Info,
    ChevronRight,
    ChevronDown
} from "lucide-react";
import Link from "next/link";
import { useRouter } from "next/navigation";

export default function LapDetailPage() {
    const router = useRouter();

    return (
        <main className="min-h-screen bg-white text-zinc-900 flex flex-col font-sans pb-safe">
            {/* Header */}
            <div className="flex items-center justify-between p-4 bg-white border-b border-gray-100 sticky top-0 z-10">
                <button onClick={() => router.back()}>
                    <ChevronLeft className="w-8 h-8 text-red-600" />
                </button>
                <div className="text-center">
                    <h1 className="text-zinc-500 font-medium">Lap 05</h1>
                </div>
                <div className="w-8 flex justify-center">
                    <MoreHorizontal className="w-6 h-6 text-red-600" />
                </div>
            </div>

            {/* Main Content */}
            <div className="flex-1 overflow-auto pb-32">

                {/* Time & Session Header */}
                <div className="py-6 text-center border-b border-gray-100">
                    <div className="flex items-center justify-between px-4 mb-2">
                        <ChevronLeft className="w-6 h-6 text-zinc-300" />
                        <span className="text-3xl font-bold font-mono text-zinc-800">01:26.37</span>
                        <ChevronRight className="w-6 h-6 text-zinc-300" />
                    </div>
                    <div className="text-xs text-zinc-400">
                        <div>Session 08</div>
                        <div>09.11.2024, 14:35</div>
                    </div>
                </div>

                {/* Metrics List */}
                <div className="divide-y divide-gray-100 px-4">
                    <MetricRow label="Max Speed (kph)" value="203.98" />
                    <MetricRow label="Min Speed (kph)" value="73.92" />
                    <MetricRow label="Avg Speed (kph)" value="129.04" />

                    <MetricRow label="Max Acceleration G" value="0.49" />
                    <MetricRow label="Max Braking G" value="1.27" />
                    <MetricRow label="Max Cornering G" value="1.38" />

                    <MetricRow label="Length" value="3.09 km" />

                    <div className="py-4 text-center">
                        <div className="text-xs text-zinc-400 mb-1">Weather</div>
                        <div className="font-bold text-zinc-700">18.9 °C, 1,023.0 hPa, 30%</div>
                    </div>

                    <div className="py-4 text-center relative">
                        <div className="text-xs text-zinc-400 mb-1">Density Altitude</div>
                        <div className="font-bold text-zinc-700">97.1 m</div>
                        <Info className="w-4 h-4 text-zinc-300 absolute right-0 top-1/2 -translate-y-1/2" />
                    </div>

                    <MetricRow label="Sector 1" value="31.62" />
                    <MetricRow label="Sector 2" value="28.45" />
                    <MetricRow label="Sector 3" value="26.30" />
                </div>
            </div>

            {/* Footer Actions */}
            <div className="fixed bottom-0 left-0 right-0 p-4 bg-white border-t border-gray-100 pb-safe flex gap-4">
                <button className="flex-1 py-3 rounded-full border border-zinc-400 text-zinc-600 font-bold text-sm hover:bg-zinc-50 transition-colors">
                    Hide
                </button>
                <Link href="/analysis/compare" className="flex-1">
                    <button className="w-full py-3 rounded-full border border-zinc-400 text-zinc-600 font-bold text-sm hover:bg-zinc-50 transition-colors">
                        Compare
                    </button>
                </Link>
            </div>
        </main>
    );
}

function MetricRow({ label, value }: { label: string, value: string }) {
    return (
        <div className="py-4 text-center">
            <div className="text-xs text-zinc-400 mb-1">{label}</div>
            <div className="text-xl font-bold font-mono text-zinc-800 tracking-tight">{value}</div>
        </div>
    );
}
