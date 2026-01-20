"use client";

import {
    ChevronLeft,
    Settings,
    Battery,
    Video,
    MapPin,
    Signal,
    Map
} from 'lucide-react';
import Link from 'next/link';
import { useState, useEffect } from 'react';
import dynamic from 'next/dynamic';

// Dynamically import SatelliteMap to avoid SSR issues
const SatelliteMap = dynamic(() => import('../components/SatelliteMap'), { ssr: false });

export default function DragMeterPage() {
    const [speed, setSpeed] = useState(0);
    const [status, setStatus] = useState<'READY' | 'ACCEL' | 'FINISH'>('READY');
    const [showMap, setShowMap] = useState(false);

    // Mock speed effect
    useEffect(() => {
        const interval = setInterval(() => {
            setSpeed(prev => {
                const noise = (Math.random() - 0.5) * 0.5;
                const newSpeed = Math.max(0, prev + noise);
                return parseFloat(newSpeed.toFixed(1));
            });
        }, 100);
        return () => clearInterval(interval);
    }, []);

    // Gauge Calculations
    const maxSpeed = 300;
    const angleRange = 260; // Total degrees of the arc
    const startAngle = -220; // Starting degree (left bottom)
    const radius = 120;
    const center = 150;

    // Normalize speed to rotation
    const rotation = startAngle + (Math.min(speed, maxSpeed) / maxSpeed) * angleRange;

    // Tick Marks Generation for Gauge
    const ticks = [];
    for (let i = 0; i <= maxSpeed; i += 20) {
        const tickAngle = startAngle + (i / maxSpeed) * angleRange;
        const rad = (tickAngle * Math.PI) / 180;
        const isMajor = i % 40 === 0;
        const innerR = isMajor ? radius - 15 : radius - 8;

        const x1 = center + Math.cos(rad) * radius;
        const y1 = center + Math.sin(rad) * radius;
        const x2 = center + Math.cos(rad) * innerR;
        const y2 = center + Math.sin(rad) * innerR;

        let labelX = 0, labelY = 0;
        if (isMajor) {
            const labelR = radius - 30;
            labelX = center + Math.cos(rad) * labelR;
            labelY = center + Math.sin(rad) * labelR;
        }

        ticks.push({ x1, y1, x2, y2, isMajor, label: isMajor ? i : null, labelX, labelY });
    }

    return (
        <div className="min-h-screen bg-background text-foreground flex flex-col">

            {/* Header (Shared) */}
            <div className={`flex items-center justify-between p-4 z-10 ${/* Landscape Header tweaks can go here */ ""}`}>
                <Link href="/devices" className="text-primary">
                    <ChevronLeft className="w-8 h-8" />
                </Link>

                <div className="text-center">
                    <div className="text-lg font-bold text-text-secondary">Drag Run 1, 1ft: ON</div>
                </div>

                <div className="flex items-center gap-3">
                    <button
                        onClick={() => setShowMap(!showMap)}
                        className={`p-2 rounded-lg transition-colors ${showMap ? 'bg-primary text-white' : 'text-primary hover:bg-primary/10'
                            }`}
                        title="Toggle Map View"
                    >
                        <Map className="w-6 h-6" />
                    </button>
                    <Video className="w-6 h-6 text-primary" />
                    {/* Landscape Icons */}
                    <div className="hidden landscape:flex items-center gap-2">
                        <Battery className="w-6 h-5 text-green-500" />
                        <MapPin className="w-5 h-5 text-text-secondary" />
                        <Signal className="w-5 h-5 text-text-secondary" />
                    </div>
                </div>
            </div>

            {/* SATELLITE MAP VIEW */}
            {showMap && (
                <div className="flex-1" style={{ minHeight: '500px' }}>
                    <SatelliteMap />
                </div>
            )}

            {/* PORTRAIT LAYOUT */}
            {!showMap && (
                <div className="flex-1 flex flex-col landscape:hidden">
                    {/* Status Bar */}
                    <div className="px-4 flex justify-between items-center text-text-secondary mb-4">
                        <div className="flex items-center gap-1 text-green-500">
                            <Battery className="w-8 h-6" />
                        </div>
                        <div className="flex items-center gap-2">
                            <MapPin className="w-5 h-5 text-text-secondary" />
                            <Signal className="w-5 h-5 text-text-secondary" />
                        </div>
                    </div>

                    {/* Gauge Display */}
                    <div className="flex-1 flex flex-col items-center justify-start relative pt-8">
                        <div className="relative w-[300px] h-[300px]">
                            <svg width="300" height="300" className="overflow-visible">
                                {ticks.map((tick, i) => (
                                    <g key={i}>
                                        <line x1={tick.x1} y1={tick.y1} x2={tick.x2} y2={tick.y2} stroke="#555" strokeWidth={tick.isMajor ? 3 : 1} />
                                        {tick.label !== null && (
                                            <text x={tick.labelX} y={tick.labelY} fill="#666" fontSize="12" textAnchor="middle" alignmentBaseline="middle">
                                                {tick.label}
                                            </text>
                                        )}
                                    </g>
                                ))}
                                <path
                                    d={`M ${center + Math.cos(startAngle * Math.PI / 180) * radius} ${center + Math.sin(startAngle * Math.PI / 180) * radius} A ${radius} ${radius} 0 0 1 ${center + Math.cos(rotation * Math.PI / 180) * radius} ${center + Math.sin(rotation * Math.PI / 180) * radius}`}
                                    fill="none" stroke="#ffa500" strokeWidth="8" strokeLinecap="round" className="drop-shadow-[0_0_8px_rgba(255,165,0,0.5)]"
                                />
                            </svg>
                            <div className="absolute inset-0 flex flex-col items-center justify-center pt-8">
                                <div className="text-6xl font-bold font-mono tracking-tighter text-foreground">
                                    {speed.toFixed(1)}
                                </div>
                                <div className="text-xl font-bold text-text-secondary uppercase">kph</div>
                            </div>
                        </div>
                        <div className="w-[80%] border border-border-color rounded px-4 py-2 mt-4 text-center">
                            <span className="text-text-secondary text-sm font-bold uppercase">Speed</span>
                        </div>
                    </div>

                    {/* Main Result Box */}
                    <div className="px-4 pb-8 w-full">
                        <div className={`bg-card-bg rounded-2xl border-2 border-primary p-4 flex justify-between items-center shadow-lg relative overflow-hidden`}>
                            <span className="font-bold text-xl text-foreground">0-100 kph</span>
                            <span className="font-bold text-3xl font-mono text-foreground">--.--</span>
                        </div>
                        <div className="flex justify-between items-center px-4 mt-4">
                            <span className="text-text-secondary font-bold">60 ft</span>
                            <span className="text-text-secondary font-mono text-lg">--.--</span>
                        </div>
                    </div>

                    <div className="px-4 pb-4 flex justify-between text-xs text-text-secondary font-mono">
                        <span>18.0 °C</span>
                        <span>DA: 781 m</span>
                    </div>
                </div>
            )}

            {/* LANDSCAPE LAYOUT */}
            {!showMap && (
                <div className="hidden landscape:flex flex-col flex-1 px-8 pb-4">
                    {/* Top Speed Area */}
                    <div className="flex justify-center mb-4">
                        <span className="text-6xl font-bold font-mono text-foreground tracking-tighter animate-pulse">
                            104.53
                        </span>
                    </div>

                    {/* Split Content */}
                    <div className="flex-1 flex gap-4">
                        {/* Left List */}
                        <div className="w-1/3 flex flex-col justify-center space-y-4 pr-4 border-r border-border-color">
                            <div className="flex justify-between items-center text-xl">
                                <span className="font-bold text-[#65a30d]">60 ft</span>
                                <span className="font-mono font-bold text-[#65a30d]">1.46</span>
                            </div>
                            <div className="flex justify-between items-center text-xl">
                                <span className="font-bold text-[#65a30d]">100 m</span>
                                <span className="font-mono font-bold text-[#65a30d]">3.91</span>
                            </div>
                            <div className="flex justify-between items-center text-xl">
                                <span className="font-bold text-[#65a30d]">200 m</span>
                                <span className="font-mono font-bold text-[#65a30d]">6.91</span>
                            </div>
                            <div className="flex justify-between items-center text-xl opacity-50">
                                <span className="font-bold text-text-secondary">400 m</span>
                                <span className="font-mono font-bold text-text-secondary">--.--</span>
                            </div>
                        </div>

                        {/* Right Highlight Box */}
                        <div className="flex-1 bg-[#4ade80] rounded-lg flex flex-col items-center justify-center text-white relative overflow-hidden">
                            <div className="text-2xl font-bold mb-2">200 m</div>
                            <div className="text-[6rem] font-mono font-bold leading-none tracking-tighter">
                                6.91
                            </div>
                            <div className="absolute bottom-4 right-6 text-xl font-bold">
                                Slope: 1.6 %
                            </div>
                        </div>
                    </div>
                </div>
            )}

        </div>
    );
}
