'use client';

import Link from 'next/link';
import { ArrowLeft, Calendar, Clock, MapPin, Gauge as Speedometer, Zap, Share2, Download, Activity } from 'lucide-react';
import { LineChart, Line, XAxis, YAxis, CartesianGrid, Tooltip, ResponsiveContainer, AreaChart, Area } from 'recharts';
import Navbar from "../../components/Navbar";
import SessionMapWrapper from "../../components/SessionMapWrapper";

interface SessionData {
    id: any;
    originalFilename: string;
    uploadDate: string;
    stats: any;
    points: any[];
}

export default function SessionView({ session }: { session: SessionData }) {
    const points = session.points;
    const id = session.id;

    const maxSpeed = session.stats?.maxSpeed?.toFixed(1) || 0;
    const maxRpm = session.stats?.maxRpm?.toFixed(0) || 0;
    const avgSpeed = (points.reduce((a: any, b: any) => a + (b.speed || 0), 0) / (points.length || 1)).toFixed(1);

    // Format Date
    const dateObj = new Date(session.uploadDate);
    const dateStr = dateObj.toLocaleDateString();
    const timeStr = dateObj.toLocaleTimeString([], { hour: '2-digit', minute: '2-digit' });

    // Map points for Recharts (needs exact keys)
    const chartData = points.map((p, idx) => ({
        timestamp: idx, // or use p.time if formatted
        speed: p.speed,
        rpm: p.rpm,
        lat: p.lat,
        lng: p.lng
    }));

    const mapPoints = points.map((p: any) => ({ lat: p.lat, lng: p.lng }));

    return (
        <div className="min-h-screen bg-gradient-to-br from-slate-900 via-blue-900 to-slate-900">
            <Navbar />

            <div className="container mx-auto px-6 py-8">
                {/* Header */}
                <div className="flex flex-col md:flex-row items-start md:items-center justify-between mb-8">
                    <div>
                        <Link href="/sessions" className="flex items-center text-slate-400 hover:text-white mb-4 transition">
                            <ArrowLeft className="w-4 h-4 mr-2" /> Back to Sessions
                        </Link>
                        <h1 className="text-3xl font-bold text-white flex items-center gap-3">
                            {session.originalFilename || "Recorded Session"}
                            <span className="text-sm bg-orange-500 text-white px-2 py-1 rounded">#{id}</span>
                        </h1>
                        <div className="flex flex-wrap gap-4 mt-2 text-slate-400 text-sm">
                            <span className="flex items-center"><Calendar className="w-4 h-4 mr-1" /> {dateStr}</span>
                            <span className="flex items-center"><Clock className="w-4 h-4 mr-1" /> {timeStr}</span>
                            <span className="flex items-center"><MapPin className="w-4 h-4 mr-1" /> Track Location</span>
                        </div>
                    </div>
                    <div className="flex gap-3 mt-4 md:mt-0">
                        <button className="bg-slate-800 hover:bg-slate-700 text-white px-4 py-2 rounded-lg transition flex items-center border border-slate-700">
                            <Share2 className="w-4 h-4 mr-2" /> Share
                        </button>
                        <button className="bg-orange-600 hover:bg-orange-700 text-white px-4 py-2 rounded-lg transition flex items-center">
                            <Download className="w-4 h-4 mr-2" /> Export GPX
                        </button>
                    </div>
                </div>

                {/* Main Grid */}
                <div className="grid lg:grid-cols-3 gap-6 mb-8">
                    {/* Left Column: Stats */}
                    <div className="space-y-6">
                        <div className="bg-slate-800/50 backdrop-blur-sm border border-slate-700 rounded-xl p-6">
                            <h2 className="text-xl font-bold text-white mb-4">Performance Stats</h2>
                            <div className="grid grid-cols-2 gap-4">
                                <StatItem label="Max Speed" value={`${maxSpeed} km/h`} icon={<Speedometer className="text-blue-500" />} />
                                <StatItem label="Max RPM" value={`${maxRpm}`} icon={<Zap className="text-orange-500" />} />
                                <StatItem label="Avg Speed" value={`${avgSpeed} km/h`} icon={<Speedometer className="text-green-500" />} />
                                <StatItem label="Points" value={`${points.length}`} icon={<Activity className="text-purple-500" />} />
                            </div>
                        </div>

                        <div className="bg-slate-800/50 backdrop-blur-sm border border-slate-700 rounded-xl p-6">
                            <h2 className="text-xl font-bold text-white mb-4">Best Lap</h2>
                            <div className="text-4xl font-mono font-bold text-green-400">--:--.---</div>
                            <div className="text-slate-400 text-sm mt-1">Lap detection pending</div>
                        </div>
                    </div>

                    {/* Right Column: Map */}
                    <div className="lg:col-span-2 h-[400px] bg-slate-800 rounded-xl overflow-hidden border border-slate-700 shadow-xl relative">
                        <SessionMapWrapper points={mapPoints} />
                        <div className="absolute top-4 right-4 bg-black/60 backdrop-blur px-3 py-1 rounded text-xs text-white z-[400]">
                            Satellite View
                        </div>
                    </div>
                </div>

                {/* Charts Section */}
                <div className="grid gap-6">
                    {/* Speed Chart */}
                    <div className="bg-slate-800/50 backdrop-blur-sm border border-slate-700 rounded-xl p-6">
                        <h3 className="text-white font-semibold mb-4 flex items-center">
                            <Speedometer className="w-5 h-5 mr-2 text-blue-500" /> Speed Telemetry
                        </h3>
                        <div className="h-[250px] w-full">
                            <ResponsiveContainer width="100%" height="100%">
                                <AreaChart data={chartData}>
                                    <defs>
                                        <linearGradient id="colorSpeed" x1="0" y1="0" x2="0" y2="1">
                                            <stop offset="5%" stopColor="#3b82f6" stopOpacity={0.3} />
                                            <stop offset="95%" stopColor="#3b82f6" stopOpacity={0} />
                                        </linearGradient>
                                    </defs>
                                    <CartesianGrid strokeDasharray="3 3" stroke="#334155" />
                                    <XAxis dataKey="timestamp" hide />
                                    <YAxis stroke="#94a3b8" />
                                    <Tooltip
                                        contentStyle={{ backgroundColor: '#1e293b', borderColor: '#334155', color: '#f8fafc' }}
                                        itemStyle={{ color: '#60a5fa' }}
                                    />
                                    <Area type="monotone" dataKey="speed" stroke="#3b82f6" fillOpacity={1} fill="url(#colorSpeed)" />
                                </AreaChart>
                            </ResponsiveContainer>
                        </div>
                    </div>

                    {/* RPM Chart */}
                    <div className="bg-slate-800/50 backdrop-blur-sm border border-slate-700 rounded-xl p-6">
                        <h3 className="text-white font-semibold mb-4 flex items-center">
                            <Zap className="w-5 h-5 mr-2 text-orange-500" /> RPM Telemetry
                        </h3>
                        <div className="h-[250px] w-full">
                            <ResponsiveContainer width="100%" height="100%">
                                <LineChart data={chartData}>
                                    <CartesianGrid strokeDasharray="3 3" stroke="#334155" />
                                    <XAxis dataKey="timestamp" hide />
                                    <YAxis stroke="#94a3b8" domain={[0, 'auto']} />
                                    <Tooltip
                                        contentStyle={{ backgroundColor: '#1e293b', borderColor: '#334155', color: '#f8fafc' }}
                                        itemStyle={{ color: '#f97316' }}
                                    />
                                    <Line type="monotone" dataKey="rpm" stroke="#f97316" strokeWidth={2} dot={false} />
                                </LineChart>
                            </ResponsiveContainer>
                        </div>
                    </div>
                </div>
            </div>
        </div>
    );
}

function StatItem({ label, value, icon }: { label: string, value: string, icon: React.ReactNode }) {
    return (
        <div className="bg-slate-900/50 p-3 rounded-lg border border-slate-700/50">
            <div className="flex items-center justify-between mb-1">
                <span className="text-slate-400 text-xs">{label}</span>
                {icon}
            </div>
            <div className="text-lg font-bold text-white">{value}</div>
        </div>
    );
}
