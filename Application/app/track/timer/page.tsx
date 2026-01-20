"use client";

import {
    ChevronLeft,
    Video,
    Battery,
    MapPin,
    Signal
} from 'lucide-react';
import Link from 'next/link';
import { useState, useEffect } from 'react';

export default function LapTimerPage() {
    const [time, setTime] = useState(0); // in ms
    const [speed, setSpeed] = useState(0);
    const [gForce, setGForce] = useState({ x: 0, y: 0 });
    const [trail, setTrail] = useState<{ x: number, y: number }[]>([]);

    // New State for Mode
    const [mode, setMode] = useState<'G-FORCE' | 'PREDICTIVE'>('PREDICTIVE'); // Default to new requested mode
    const [predGap, setPredGap] = useState(-2.47);

    // Timer & Physics effect
    useEffect(() => {
        const start = Date.now();
        const interval = setInterval(() => {
            const now = Date.now();
            setTime(now - start);

            // Mock Speed
            setSpeed(prev => {
                const noise = (Math.random() - 0.5) * 5;
                return Math.max(0, Math.min(300, prev + noise + (Math.random() > 0.5 ? 1 : -0.5)));
            });

            // Mock G-Force
            const t = now / 1000;
            const newG = {
                x: Math.cos(t) * 0.8 + (Math.random() - 0.5) * 0.2,
                y: Math.sin(t * 1.3) * 0.8 + (Math.random() - 0.5) * 0.2
            };
            setGForce(newG);
            setTrail(prev => [...prev.slice(-20), newG]);

            // Mock Predictive Gap
            setPredGap(prev => {
                // Slow drift
                return Math.max(-10, Math.min(10, prev + (Math.random() - 0.5) * 0.1));
            });

        }, 50);
        return () => clearInterval(interval);
    }, []);

    // Format time MM:SS.ms
    const formatTime = (ms: number) => {
        const mins = Math.floor(ms / 60000).toString().padStart(2, '0');
        const secs = Math.floor((ms % 60000) / 1000).toString().padStart(2, '0');
        const centis = Math.floor((ms % 1000) / 10).toString().padStart(2, '0');
        return `${mins}:${secs}.${centis}`;
    };

    // Gauge Calculations
    const maxSpeed = 280;
    const angleRange = 270;
    const startAngle = -225;
    const radius = 140;
    const center = 160;
    const rotation = startAngle + (Math.min(speed, maxSpeed) / maxSpeed) * angleRange;

    // Ticks
    const ticks = [];
    for (let i = 0; i <= maxSpeed; i += 20) {
        const angle = startAngle + (i / maxSpeed) * angleRange;
        const rad = (angle * Math.PI) / 180;
        const isMajor = i % 40 === 0;
        const innerR = isMajor ? radius - 15 : radius - 8;
        const x1 = center + Math.cos(rad) * radius;
        const y1 = center + Math.sin(rad) * radius;
        const x2 = center + Math.cos(rad) * innerR;
        const y2 = center + Math.sin(rad) * innerR;

        // Optimize labels: only calculate if major
        let labelX = 0, labelY = 0;
        if (isMajor) {
            const labelR = radius - 35;
            labelX = center + Math.cos(rad) * labelR;
            labelY = center + Math.sin(rad) * labelR;
        }

        ticks.push({ x1, y1, x2, y2, isMajor, label: isMajor ? i : null, labelX, labelY });
    }

    return (
        <main className="min-h-screen bg-neutral-100 text-zinc-900 flex flex-col font-sans pb-safe">
            {/* Header */}
            <div className="flex items-center justify-between p-2 pt-safe z-10">
                <Link href="/track" className="p-2">
                    <ChevronLeft className="w-8 h-8 text-red-600" />
                </Link>
                <div className="text-zinc-500 font-medium">San Martino del Lago</div>
                <div className="flex items-center gap-2">
                    <Video className="w-6 h-6 text-red-600" />
                    <Battery className="w-6 h-4 text-green-600 rotate-90" />
                    <MapPin className="w-5 h-5 text-zinc-500" />
                    <Signal className="w-4 h-4 text-zinc-500" />
                </div>
            </div>

            {/* Gauge Section */}
            <div className="flex-1 flex flex-col items-center justify-center relative -mt-8">
                <div className="relative w-[320px] h-[320px]">
                    <svg width="320" height="320" className="overflow-visible">

                        {/* Common: Gauge Ticks */}
                        {ticks.map((t, i) => (
                            <g key={i}>
                                <line x1={t.x1} y1={t.y1} x2={t.x2} y2={t.y2} stroke="#999" strokeWidth={t.isMajor ? 2 : 1} />
                                {t.label !== null && <text x={t.labelX} y={t.labelY} fill="#666" fontSize="10" textAnchor="middle" alignmentBaseline="middle">{t.label}</text>}
                            </g>
                        ))}

                        {/* Mode Specific Inner Content */}
                        {mode === 'G-FORCE' ? (
                            <>
                                {/* G-Force Grid */}
                                <g transform={`translate(${center}, ${center})`} opacity="0.5">
                                    <circle r="40" fill="none" stroke="#ccc" strokeDasharray="4 2" />
                                    <circle r="80" fill="none" stroke="#ccc" strokeDasharray="4 2" />
                                    <line x1="-90" y1="0" x2="90" y2="0" stroke="#999" />
                                    <line x1="0" y1="-90" x2="0" y2="90" stroke="#999" />

                                    <text x="82" y="-2" fontSize="10" fill="#666">1.0g</text>
                                    <text x="42" y="-2" fontSize="10" fill="#666">0.5g</text>
                                </g>

                                {/* G-Force Plot */}
                                <g transform={`translate(${center}, ${center})`}>
                                    {trail.map((pt, i) => (
                                        <circle key={i} cx={pt.x * 80} cy={-pt.y * 80} r={2 + i / 5} fill="red" opacity={i / trail.length * 0.3} />
                                    ))}
                                    <circle cx={gForce.x * 80} cy={-gForce.y * 80} r="6" fill="#ef4444" stroke="white" strokeWidth="2" />
                                </g>
                            </>
                        ) : (
                            <>
                                {/* Predictive Circle */}
                                <circle
                                    cx={center}
                                    cy={center}
                                    r={85}
                                    fill={predGap < 0 ? "#65a30d" : "#dc2626"} // Green-600 : Red-600
                                    className="transition-colors duration-300"
                                />
                            </>
                        )}

                        {/* Common: Speed Arc */}
                        <path
                            d={`M ${center + Math.cos(startAngle * Math.PI / 180) * radius} ${center + Math.sin(startAngle * Math.PI / 180) * radius} A ${radius} ${radius} 0 0 1 ${center + Math.cos(rotation * Math.PI / 180) * radius} ${center + Math.sin(rotation * Math.PI / 180) * radius}`}
                            fill="none" stroke="#ffa500" strokeWidth="8" strokeLinecap="round" className="drop-shadow-md"
                        />
                    </svg>

                    {/* Overlay Text for Predictive */}
                    {mode === 'PREDICTIVE' && (
                        <div className="absolute inset-0 flex flex-col items-center justify-center pointer-events-none pb-2">
                            <div className="text-4xl font-bold font-mono text-white drop-shadow-sm">
                                {predGap > 0 ? '+' : ''}{predGap.toFixed(2)}
                            </div>
                            <div className="text-white text-xs font-bold mt-1 drop-shadow-sm">
                                Target: 01:43.21
                            </div>
                        </div>
                    )}

                    {/* Toggle Label */}
                    <div className="absolute bottom-16 left-0 right-0 text-center">
                        <button
                            onClick={() => setMode(prev => prev === 'G-FORCE' ? 'PREDICTIVE' : 'G-FORCE')}
                            className="bg-white border border-zinc-400 px-3 py-1 inline-block rounded text-xs font-bold text-zinc-600 shadow-sm active:scale-95 transition-transform"
                        >
                            {mode === 'G-FORCE' ? 'G-Forces' : 'Predictive Gap'}
                        </button>
                    </div>

                    <div className="absolute bottom-12 right-0 text-xs text-zinc-500 font-bold text-right leading-tight">
                        DA:<br />513 m
                    </div>
                    <div className="absolute bottom-12 left-0 text-xs text-zinc-500 font-bold leading-tight">
                        10.6 °C
                    </div>
                </div>
            </div>

            {/* Lap Timer Card */}
            <div className="px-4 pb-8 space-y-2">
                <div className="bg-white rounded-xl border-2 border-red-500 p-4 flex justify-between items-center shadow-sm">
                    <span className="text-zinc-400 font-bold text-lg uppercase">LAP 02</span>
                    <span className="text-5xl font-bold text-zinc-700 font-mono tracking-tighter">
                        {formatTime(time)}
                    </span>
                </div>

                <div className="flex justify-between px-4 text-zinc-500 font-medium">
                    <span>Lap 01</span>
                    <span className="font-mono">01:43.21</span>
                </div>
            </div>

        </main>
    );
}
