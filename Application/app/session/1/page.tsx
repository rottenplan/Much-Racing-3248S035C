"use client";

import {
    ChevronLeft,
    MoreHorizontal,
    ChevronRight,
    Cloud,
    Thermometer,
    Gauge,
    Wind,
    Share2,
    Video
} from "lucide-react";
import Link from "next/link";
import { useRouter } from "next/navigation";

export default function SessionPage() {
    const router = useRouter();

    return (
        <main className="min-h-screen pb-20 bg-background text-foreground">
            {/* Header */}
            <header className="sticky top-0 z-10 bg-background/80 backdrop-blur-md border-b border-border-color flex items-center justify-between px-4 h-16">
                <button onClick={() => router.back()} className="p-1">
                    <ChevronLeft className="w-6 h-6 text-primary" />
                </button>
                <h1 className="text-xl font-bold font-mono">Session 02</h1>
                <MoreHorizontal className="w-6 h-6 text-primary" />
            </header>

            <div className="p-4 space-y-4">
                {/* Info Card */}
                <div className="bg-card-bg rounded-xl shadow-sm border border-border-color p-4">
                    <div className="flex justify-between items-start mb-2">
                        <div>
                            <div className="font-bold text-foreground">23.10.2024, 15:12</div>
                            <div className="text-sm text-text-secondary">2 Runs • Audi RS3</div>
                        </div>
                        <div className="text-right">
                            <div className="text-xs text-text-secondary uppercase tracking-wider">ROLLOUT</div>
                            <div className="font-bold text-highlight font-mono">1 FT</div>
                        </div>
                    </div>

                    <div className="grid grid-cols-2 gap-4 mt-4 py-4 border-t border-border-color">
                        <div className="flex items-center gap-3">
                            <div className="p-2 bg-white/5 rounded-lg">
                                <Video className="w-5 h-5 text-primary" />
                            </div>
                            <div>
                                <div className="text-xs text-text-secondary">Video</div>
                                <div className="font-bold text-sm">10 clips</div>
                            </div>
                        </div>
                        <div className="flex items-center gap-3">
                            <div className="p-2 bg-white/5 rounded-lg">
                                <Share2 className="w-5 h-5 text-highlight" />
                            </div>
                            <div>
                                <div className="text-xs text-text-secondary">Shared By</div>
                                <div className="font-bold text-sm">Alex K.</div>
                            </div>
                        </div>
                    </div>
                </div>

                {/* Weather / DA Card (New) */}
                <div className="bg-card-bg rounded-xl shadow-sm border border-border-color p-4">
                    <div className="text-xs font-bold text-text-secondary uppercase mb-3">Conditions</div>
                    <div className="grid grid-cols-4 gap-2">
                        <WeatherStat icon={Thermometer} label="Temp" value="18°C" />
                        <WeatherStat icon={Cloud} label="Press" value="1012" />
                        <WeatherStat icon={Wind} label="Wind" value="2 m/s" />
                        <WeatherStat icon={Gauge} label="DA" value="+412m" />
                    </div>
                </div>

                {/* Graph Placeholder (New) */}
                <div className="bg-card-bg rounded-xl shadow-sm border border-border-color p-4 h-48 flex items-center justify-center relative overflow-hidden group">
                    {/* Mock Graph Lines */}
                    <svg className="absolute inset-0 w-full h-full opacity-30" preserveAspectRatio="none">
                        <path d="M0,150 Q50,140 100,100 T200,50 T300,20" fill="none" stroke="var(--primary)" strokeWidth="2" />
                        <path d="M0,150 L300,150" fill="none" stroke="#333" strokeWidth="1" />
                    </svg>
                    <div className="text-center z-10">
                        <p className="text-text-secondary text-sm">Speed / G-Force Analysis</p>
                        <button className="mt-2 px-4 py-1 bg-primary text-white text-xs font-bold rounded-full">View Full Graph</button>
                    </div>
                </div>

                {/* Results List */}
                <div className="space-y-2">
                    <div className="flex justify-between items-center px-1">
                        <h3 className="text-lg font-bold">Best Results</h3>
                        <div className="flex gap-2 text-xs font-bold">
                            <span className="text-text-secondary cursor-pointer">All</span>
                            <span className="text-primary border-b border-primary">Best</span>
                        </div>
                    </div>

                    <div className="bg-card-bg rounded-xl border border-border-color overflow-hidden">
                        <ResultRow label="60 ft" value="2.60s" sub="@ 50.17 km/h" isBest={false} />
                        <ResultRow label="1/4 mile" value="13.18s" sub="@ 184.49 km/h" isBest={true} />
                        <ResultRow label="0-100 km/h" value="5.19s" sub="75.0 m" isBest={false} />
                        <ResultRow label="100-200 km/h" value="10.42s" sub="460.0 m" isBest={true} />
                    </div>
                </div>

            </div>
        </main>
    );
}

function WeatherStat({ icon: Icon, label, value }: { icon: any, label: string, value: string }) {
    return (
        <div className="flex flex-col items-center p-2 bg-background rounded-lg border border-border-color">
            <Icon className="w-4 h-4 text-text-secondary mb-1" />
            <span className="text-[10px] text-text-secondary uppercase">{label}</span>
            <span className="text-sm font-bold font-mono">{value}</span>
        </div>
    )
}

function ResultRow({ label, value, sub, isBest }: { label: string, value: string, sub: string, isBest: boolean }) {
    return (
        <div className="p-4 border-b border-border-color last:border-0 hover:bg-white/5 cursor-pointer transition-colors flex justify-between items-center">
            <div>
                <div className="text-foreground font-medium">{label}</div>
                <div className="text-xs text-text-secondary">{sub}</div>
            </div>
            <div className="flex items-center gap-3">
                {isBest && <span className="bg-highlight/10 text-highlight text-[10px] px-1.5 py-0.5 rounded font-bold uppercase">PB</span>}
                <div className={`font-mono font-bold text-xl ${isBest ? 'text-highlight' : 'text-foreground'}`}>
                    {value}
                </div>
                <ChevronRight className="w-4 h-4 text-text-secondary" />
            </div>
        </div>
    )
}
