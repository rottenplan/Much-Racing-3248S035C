"use client";

import { useEffect, useState } from 'react';
import { Gauge, Zap, Activity } from 'lucide-react';

interface SpeedometerProps {
    initialSpeed?: number;
    maxSpeed?: number;
}

export default function Speedometer({ initialSpeed = 0, maxSpeed = 200 }: SpeedometerProps) {
    const [speed, setSpeed] = useState(initialSpeed);
    const [rpm, setRpm] = useState(0);
    const [gear, setGear] = useState(1);

    // Mock live data loop
    useEffect(() => {
        const interval = setInterval(() => {
            // Fluctuate speed randomly around a target for demo
            const baseSpeed = 85;
            const noise = (Math.random() - 0.5) * 10;
            let newSpeed = baseSpeed + noise;
            if (newSpeed < 0) newSpeed = 0;
            if (newSpeed > maxSpeed) newSpeed = maxSpeed;

            setSpeed(prev => {
                const delta = newSpeed - prev;
                return prev + delta * 0.1; // Smooth transition
            });

            // Mock RPM based on speed
            const newRpm = (newSpeed / maxSpeed) * 12000 + Math.random() * 500;
            setRpm(newRpm);

            // Mock Gear
            if (newSpeed === 0) setGear(0); // N
            else if (newSpeed < 40) setGear(1);
            else if (newSpeed < 80) setGear(2);
            else if (newSpeed < 120) setGear(3);
            else setGear(4);

        }, 100);

        return () => clearInterval(interval);
    }, [maxSpeed]);

    // SVG Gauge Calculations
    const radius = 80;
    const stroke = 12;
    const normalizedRadius = radius - stroke * 2;
    const circumference = normalizedRadius * 2 * Math.PI;
    const strokeDashoffset = circumference - (speed / maxSpeed) * circumference;

    return (
        <div className="flex items-center justify-center p-4">
            <div className="relative w-64 h-64 flex items-center justify-center">

                {/* Outer Ring */}
                <div className="absolute inset-0 rounded-full border-4 border-background-secondary opacity-30"></div>

                {/* SVG Gauge */}
                <svg
                    height={radius * 2}
                    width={radius * 2}
                    className="absolute transform -rotate-90"
                >
                    <circle
                        stroke="currentColor"
                        fill="transparent"
                        strokeWidth={stroke}
                        strokeDasharray={circumference + ' ' + circumference}
                        style={{ strokeDashoffset, strokeLinecap: 'round' }}
                        stroke-width={stroke}
                        r={normalizedRadius}
                        cx={radius}
                        cy={radius}
                        className="text-primary transition-all duration-100 ease-linear"
                    />
                </svg>

                {/* Center Digital Display */}
                <div className="absolute flex flex-col items-center">
                    <span className="text-5xl font-data font-bold text-foreground tabular-nums">
                        {Math.round(speed)}
                    </span>
                    <span className="text-xs font-racing text-text-secondary">KM/H</span>

                    <div className="mt-2 px-2 py-0.5 bg-background-secondary rounded text-xs font-mono text-highlight">
                        GEAR {gear === 0 ? 'N' : gear}
                    </div>
                </div>

                {/* RPM Bar (Bottom) */}
                <div className="absolute bottom-4 w-48 h-2 bg-background-secondary rounded-full overflow-hidden">
                    <div
                        className="h-full bg-gradient-to-r from-success via-warning to-error transition-all duration-100 ease-linear"
                        style={{ width: `${(rpm / 12000) * 100}%` }}
                    ></div>
                </div>
                <div className="absolute bottom-0 text-[10px] text-text-secondary font-mono">
                    {Math.round(rpm)} RPM
                </div>
            </div>
        </div>
    );
}
