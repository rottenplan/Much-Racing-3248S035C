'use client';

import Link from 'next/link';
import { ArrowLeft, MapPin, Clock, TrendingUp, Play, Pause, Maximize, Download, Map as MapIcon, BarChart3, Settings } from 'lucide-react';
import { useParams } from 'next/navigation';
import { useState, useMemo } from 'react';
import {
  AreaChart,
  Area,
  XAxis,
  YAxis,
  CartesianGrid,
  Tooltip,
  ResponsiveContainer,
  ReferenceLine
} from 'recharts';

// Mock session data
const sessionData: any = {
  '1': {
    id: 1,
    track: 'Sentul Karting Circuit',
    location: 'Bogor, Indonesia',
    date: '2026-01-06',
    time: '14:30:47',
    driver: 'FARIS',
    category: 'Vespa Tune Up',
    totalLaps: 25,
    bestLap: '48.234',
    avgLap: '52.891',
    totalTime: '22:02.275',
    laps: [
      { num: 1, time: '52.891', speed: 78.5, rpm: 8200, delta: '+4.657', waterTemp: 85, egt: 620 },
      { num: 2, time: '51.234', speed: 82.1, rpm: 8450, delta: '+3.000', waterTemp: 87, egt: 635 },
      { num: 3, time: '48.234', speed: 85.3, rpm: 8600, delta: '0.000', waterTemp: 89, egt: 645 },
      { num: 4, time: '49.123', speed: 84.2, rpm: 8550, delta: '+0.889', waterTemp: 90, egt: 642 },
      { num: 5, time: '50.456', speed: 81.7, rpm: 8400, delta: '+2.222', waterTemp: 91, egt: 638 },
    ],
  },
};

// Helper to generate mock telemetry data for a lap
const generateMockTelemetry = (lapData: any) => {
  const points = 100;
  const data = [];
  const baseSpeed = lapData.speed;
  const baseRpm = lapData.rpm;

  for (let i = 0; i < points; i++) {
    // Create a fake track profile (straight, curve, straight, tight curve, straight)
    let speedMod = 1.0;
    let rpmMod = 1.0;
    
    if (i < 20) { // Straight 1
      speedMod = 0.5 + (i / 20) * 0.7; // Accel
      rpmMod = 0.6 + (i / 20) * 0.4;
    } else if (i < 35) { // Curve 1
      speedMod = 0.8 - ((i - 20) / 15) * 0.3; // Brake
      rpmMod = 1.0 - ((i - 20) / 15) * 0.4;
    } else if (i < 65) { // Straight 2
      speedMod = 0.5 + ((i - 35) / 30) * 0.8; // Accel hard
      rpmMod = 0.6 + ((i - 35) / 30) * 0.4;
    } else if (i < 80) { // Hairpin
      speedMod = 1.3 - ((i - 65) / 15) * 0.9; // Brake hard
      rpmMod = 1.0 - ((i - 65) / 15) * 0.7;
    } else { // Finish Straight
      speedMod = 0.4 + ((i - 80) / 20) * 0.8;
      rpmMod = 0.3 + ((i - 80) / 20) * 0.7;
    }

    // Add some noise
    const noise = Math.random() * 2 - 1;

    data.push({
      distance: i * 10, // fake distance meters
      speed: Math.max(0, (baseSpeed * speedMod) + noise).toFixed(1),
      rpm: Math.max(0, (baseRpm * rpmMod) + (noise * 10)).toFixed(0),
      delta: (Math.sin(i / 10) * 0.5).toFixed(3),
      waterTemp: lapData.waterTemp + (i * 0.05),
      egt: lapData.egt + (Math.random() * 5),
    });
  }
  return data;
};


export default function SessionDetailPage() {
  const params = useParams();
  const sessionId = params.id as string;
  const session = sessionData[sessionId];
  const [isLapDetailsExpanded, setIsLapDetailsExpanded] = useState(false);

  // Calculate statistics for the table
  const lapStats = session.laps.map((lap: any) => ({
    ...lap,
    minSpeed: (lap.speed * 0.6).toFixed(1), // Mock data logic
    maxSpeed: (lap.speed * 1.3).toFixed(1),
    avgSpeed: lap.speed,
    minRpm: lap.rpm - 2500,
    maxRpm: lap.rpm + 800,
    avgRpm: lap.rpm
  }));

  const [selectedLaps, setSelectedLaps] = useState<number[]>([3]); // Best lap selected by default
  const [isPlaying, setIsPlaying] = useState(false);
  const [viewMode, setViewMode] = useState<'map' | 'data'>('map'); 

  const toggleLapSelection = (lapNum: number) => {
    // Verify user doesn't unselect the last one or logic to handle empty state
    setSelectedLaps(prev => {
        if (prev.includes(lapNum)) {
            // Allow deselecting unless it's the only one? let's allow empty for now
            return prev.filter(n => n !== lapNum);
        } else {
             // For simplicity in this mock, we only show ONE selected lap's telemetry at high fidelity
             // Or we could compare two. Let's stick to single selection for the main detailed chart for now to keep it simple,
             // OR allow multiple and user sees they overlap.
             // The Recharts setup below expects a single array of data. 
             // To support multiple laps comparison, we'd need to merge datasets.
             // Let's force single selection for the Detailed Data view to mimic "Focus" mode, 
             // or just update the "active" lap data based on the LAST selected lap.
             return [lapNum]; // Single select mode for clarity in this demo
        }
    });
  };

  // Generate telemetry for the currently selected lap (or the first one if multiple)
  const currentLapId = selectedLaps.length > 0 ? selectedLaps[0] : 3;
  const currentLapData = session.laps.find((l: any) => l.num === currentLapId);
  const telemetryData = useMemo(() => generateMockTelemetry(currentLapData), [currentLapData]);

  return (
    <div className="min-h-screen bg-gradient-to-br from-slate-900 via-slate-900 to-slate-950 text-slate-200 font-sans">
      {/* Navigation */}
      <nav className="bg-slate-900/80 backdrop-blur-md border-b border-white/10 sticky top-0 z-50">
        <div className="container mx-auto px-6 py-4">
          <div className="flex items-center justify-between">
            <Link href="/" className="flex items-center space-x-3 group">
              <div className="w-10 h-10 relative transition-transform group-hover:scale-105">
                <img 
                  src="/logo.png" 
                  alt="Much Racing Logo" 
                  className="w-full h-full object-contain"
                />
              </div>
              <span className="text-white text-xl font-bold tracking-tight">Much Racing</span>
            </Link>
            <div className="hidden md:flex space-x-8">
              <Link href="/dashboard" className="text-slate-300 hover:text-white transition font-medium">Dashboard</Link>
              <Link href="/tracks" className="text-slate-300 hover:text-white transition font-medium">Tracks</Link>
              <Link href="/sessions" className="text-white font-medium border-b-2 border-orange-500 pb-1">Sessions</Link>
              <Link href="/device" className="text-slate-300 hover:text-white transition font-medium">Device</Link>
            </div>
          </div>
        </div>
      </nav>

      {/* Content */}
      <div className="container mx-auto px-6 py-8">
        {/* Breadcrumb / Back */}
        <div className="flex items-center justify-between mb-8">
            <Link href="/sessions" className="inline-flex items-center text-slate-400 hover:text-white transition group">
            <ArrowLeft className="w-4 h-4 mr-2 group-hover:-translate-x-1 transition-transform" />
            Back to Sessions
            </Link>
            <div className="text-slate-500 text-sm font-mono">Session ID: #{session.id}</div>
        </div>

        {/* Session Header Card */}
        <div className="bg-slate-800/40 backdrop-blur-sm border border-white/10 rounded-2xl p-6 mb-6 shadow-xl relative overflow-hidden">
          <div className="absolute top-0 right-0 p-3 opacity-10">
              <MapIcon className="w-64 h-64 text-white" />
          </div>
          <div className="relative z-10 flex flex-col md:flex-row md:items-center md:justify-between gap-6">
            <div>
              <div className="flex items-center space-x-3 mb-2">
                 <h1 className="text-3xl font-bold text-white tracking-tight">{session.track}</h1>
                 <span className="bg-orange-500/20 text-orange-400 text-xs px-2 py-1 rounded border border-orange-500/30 uppercase tracking-wider font-semibold">{session.category}</span>
              </div>
              <div className="flex flex-wrap items-center gap-4 text-slate-400">
                <div className="flex items-center">
                    <MapPin className="w-4 h-4 mr-1.5 text-slate-500" />
                    {session.location}
                </div>
                <div className="flex items-center">
                    <Clock className="w-4 h-4 mr-1.5 text-slate-500" />
                    {session.date} &bull; {session.time}
                </div>
              </div>
            </div>
            
            <div className="flex items-center gap-8 bg-slate-900/50 p-4 rounded-xl border border-white/5">
                <div className="text-right">
                    <div className="text-xs text-slate-500 uppercase tracking-wider font-semibold mb-1">Best Lap</div>
                    <div className="text-4xl font-black text-transparent bg-clip-text bg-gradient-to-r from-orange-400 to-red-500 font-mono">
                        {session.bestLap}
                    </div>
                </div>
                <div className="h-10 w-px bg-white/10"></div>
                <div>
                     <div className="text-xs text-slate-500 uppercase tracking-wider font-semibold mb-1">Total Laps</div>
                     <div className="text-2xl font-bold text-white">{session.totalLaps}</div>
                </div>
                <div>
                     <div className="text-xs text-slate-500 uppercase tracking-wider font-semibold mb-1">Total Time</div>
                     <div className="text-2xl font-bold text-white">{session.totalTime}</div>
                </div>
            </div>
          </div>
        </div>

        {/* Toolbar & View Toggle */}
        <div className="flex flex-col md:flex-row items-center justify-between gap-4 mb-6">
            <div className="flex items-center bg-slate-800/80 rounded-lg p-1 border border-white/10">
                 <button
                  onClick={() => setIsPlaying(!isPlaying)}
                  className="p-2 hover:bg-white/10 rounded-md text-slate-300 hover:text-white transition"
                  title="Play/Pause"
                >
                  {isPlaying ? <Pause className="w-5 h-5 fill-current" /> : <Play className="w-5 h-5 fill-current" />}
                </button>
                <div className="w-px h-4 bg-white/10 mx-1"></div>
                 <div className="flex items-center px-3 space-x-2">
                    <span className="text-xs text-slate-400 font-mono uppercase">Playback speed</span>
                    <span className="text-xs font-bold text-orange-400">1.0x</span>
                </div>
            </div>

            <div className="flex items-center space-x-3">
                 <div className="flex bg-slate-900/80 rounded-lg p-1 border border-white/10">
                    <button
                      onClick={() => setViewMode('map')}
                      className={`flex items-center space-x-2 px-4 py-2 rounded-md transition-all ${
                        viewMode === 'map'
                          ? 'bg-slate-700 shadow-md text-white ring-1 ring-white/20'
                          : 'text-slate-400 hover:text-white hover:bg-white/5'
                      }`}
                    >
                      <MapIcon className="w-4 h-4" />
                      <span className="text-sm font-semibold">MAP</span>
                    </button>
                    <button
                      onClick={() => setViewMode('data')}
                      className={`flex items-center space-x-2 px-4 py-2 rounded-md transition-all ${
                        viewMode === 'data'
                          ? 'bg-slate-700 shadow-md text-white ring-1 ring-white/20'
                          : 'text-slate-400 hover:text-white hover:bg-white/5'
                      }`}
                    >
                      <BarChart3 className="w-4 h-4" />
                      <span className="text-sm font-semibold">DATA</span>
                    </button>
                  </div>
                  <button className="p-2 bg-slate-800/80 border border-white/10 rounded-lg text-slate-400 hover:text-white hover:bg-slate-700 transition">
                    <Settings className="w-5 h-5" />
                  </button>
                  <button className="p-2 bg-slate-800/80 border border-white/10 rounded-lg text-slate-400 hover:text-white hover:bg-slate-700 transition">
                    <Download className="w-5 h-5" />
                  </button>
            </div>
        </div>

        <div className="grid lg:grid-cols-12 gap-6">
          {/* LEFT Sidebar: Lap List */}
          <div className="lg:col-span-3 space-y-4">
            <div className="bg-slate-800/40 backdrop-blur-sm border border-white/10 rounded-xl flex flex-col h-[600px] overflow-hidden">
               <div className="p-4 border-b border-white/10 bg-slate-900/30">
                 <h2 className="text-sm font-bold text-slate-300 uppercase tracking-wider">Laps Session</h2>
               </div>
               <div className="flex-1 overflow-y-auto scrollbar-thin scrollbar-thumb-slate-600 scrollbar-track-transparent p-2 space-y-1">
                  {session.laps.map((lap: any) => (
                    <div
                      key={lap.num}
                      onClick={() => toggleLapSelection(lap.num)}
                      className={`group p-3 rounded-lg cursor-pointer transition border relative overflow-hidden ${
                        selectedLaps.includes(lap.num)
                          ? 'bg-orange-500/10 border-orange-500/50'
                          : 'bg-transparent border-transparent hover:bg-white/5'
                      }`}
                    >
                      {/* Left border indicator for selected */}
                      {selectedLaps.includes(lap.num) && (
                         <div className="absolute left-0 top-0 bottom-0 w-1 bg-orange-500"></div>
                      )}
                      
                      <div className="flex items-center justify-between mb-1 pl-2">
                         <span className={`text-sm font-medium ${selectedLaps.includes(lap.num) ? 'text-white' : 'text-slate-400 group-hover:text-slate-200'}`}>
                             Lap {lap.num}
                         </span>
                         <span className={`font-mono font-bold ${
                             lap.num === 3 ? 'text-orange-400' : 'text-slate-200'
                         }`}>
                             {lap.time}
                         </span>
                      </div>
                      <div className="flex items-center justify-between text-xs pl-2">
                         <div className="flex items-center space-x-2">
                             <span className="text-slate-500">{lap.maxSpeed} km/h</span> 
                         </div>
                         <span className={`font-mono ${
                            lap.delta.startsWith('+') ? 'text-red-400' : 'text-green-400'
                         }`}>
                             {lap.delta}
                         </span>
                      </div>
                    </div>
                  ))}
               </div>
            </div>
            
            {/* Laps Details Button */}
             <button
                onClick={() => setIsLapDetailsExpanded(!isLapDetailsExpanded)}
                className="w-full bg-slate-800 hover:bg-slate-700 border border-white/10 text-white p-4 rounded-xl transition flex items-center justify-between shadow-lg group"
              >
                <div className="flex items-center">
                    <TrendingUp className="w-5 h-5 mr-3 text-orange-500" />
                    <span className="font-bold text-sm">Lap Details Table</span>
                </div>
                <div className={`bg-slate-900 p-1 rounded transition-transform duration-300 ${isLapDetailsExpanded ? 'rotate-180' : ''}`}>
                    <ArrowLeft className="-rotate-90 w-4 h-4 text-slate-400" />
                </div>
              </button>
          </div>

          {/* RIGHT Content Area */}
          <div className="lg:col-span-9 space-y-6">
            
            {/* New Expandable Lap Stats Table */}
            {isLapDetailsExpanded && (
               <div className="bg-slate-800/40 backdrop-blur-sm border border-white/10 rounded-xl p-6 shadow-2xl animate-in slide-in-from-top-4 duration-300">
                 <div className="flex items-center justify-between mb-4">
                     <h2 className="text-lg font-bold text-white">Detailed Lap Statistics</h2>
                     <button className="text-xs text-slate-400 hover:text-white underline">Export CSV</button>
                 </div>
                 <div className="overflow-x-auto">
                   <table className="w-full text-sm text-left text-slate-300">
                     <thead className="text-xs text-slate-400 uppercase bg-slate-900/50">
                       <tr>
                         <th className="px-4 py-3 rounded-tl-lg">Lap</th>
                         <th className="px-4 py-3 text-center border-l border-white/5" colSpan={3}>Speed (Km/h)</th>
                         <th className="px-4 py-3 text-center border-l border-white/5 rounded-tr-lg" colSpan={3}>RPM</th>
                       </tr>
                       <tr className="border-b border-white/5">
                         <th className="px-4 py-2"></th>
                         <th className="px-4 py-2 text-center text-blue-400 border-l border-white/5">Min</th>
                         <th className="px-4 py-2 text-center text-green-400">Max</th>
                         <th className="px-4 py-2 text-center text-orange-400">Avg</th>
                         <th className="px-4 py-2 text-center text-blue-400 border-l border-white/5">Min</th>
                         <th className="px-4 py-2 text-center text-green-400">Max</th>
                         <th className="px-4 py-2 text-center text-orange-400">Avg</th>
                       </tr>
                     </thead>
                     <tbody className="divide-y divide-white/5">
                       {lapStats.map((lap: any) => (
                         <tr key={lap.num} className={`hover:bg-white/5 transition-colors ${selectedLaps.includes(lap.num) ? 'bg-orange-500/5' : ''}`}>
                           <td className="px-4 py-3 font-semibold text-white">Lap {lap.num}</td>
                           <td className="px-4 py-3 text-center border-l border-white/5 font-mono text-slate-400">{lap.minSpeed}</td>
                           <td className="px-4 py-3 text-center font-mono text-white font-bold">{lap.maxSpeed}</td>
                           <td className="px-4 py-3 text-center font-mono text-slate-300">{lap.avgSpeed}</td>
                           <td className="px-4 py-3 text-center border-l border-white/5 font-mono text-slate-400">{lap.minRpm}</td>
                           <td className="px-4 py-3 text-center font-mono text-white font-bold">{lap.maxRpm}</td>
                           <td className="px-4 py-3 text-center font-mono text-slate-300">{lap.avgRpm}</td>
                         </tr>
                       ))}
                     </tbody>
                   </table>
                 </div>
               </div>
            )}

            {viewMode === 'map' ? (
              /* MAP MODE */
              <>
                <div className="bg-slate-900 rounded-xl border border-white/10 relative overflow-hidden h-[500px] flex items-center justify-center group">
                  {/* Grid Background */}
                  <div className="absolute inset-0 bg-[linear-gradient(rgba(255,255,255,0.03)_1px,transparent_1px),linear-gradient(90deg,rgba(255,255,255,0.03)_1px,transparent_1px)] bg-[size:40px_40px]"></div>
                  
                  {/* Map Controls */}
                  <div className="absolute top-4 right-4 flex flex-col space-y-2 z-10">
                      <div className="bg-slate-800/90 backdrop-blur border border-white/10 rounded-lg p-1 flex flex-col text-slate-400">
                          <button className="p-2 hover:text-white hover:bg-white/5 rounded transition" title="Zoom In"><Maximize className="w-4 h-4" /></button>
                          <button className="p-2 hover:text-white hover:bg-white/5 rounded transition" title="Center"><MapIcon className="w-4 h-4" /></button>
                      </div>
                      <div className="bg-slate-800/90 backdrop-blur border border-white/10 rounded-lg p-2 text-xs font-mono text-slate-400">
                         <div>Lat: -6.5432</div>
                         <div>Lon: 106.876</div>
                      </div>
                  </div>

                  {/* SVG Map Visualization */}
                  <div className="relative w-full h-full p-12 transition-transform duration-500">
                      <svg viewBox="0 0 400 300" className="w-full h-full drop-shadow-[0_0_15px_rgba(249,115,22,0.6)]">
                         <defs>
                             <linearGradient id="trackGradient" x1="0%" y1="0%" x2="100%" y2="0%">
                                 <stop offset="0%" stopColor="#f97316" />
                                 <stop offset="50%" stopColor="#ef4444" />
                                 <stop offset="100%" stopColor="#f97316" />
                             </linearGradient>
                             <filter id="glow">
                                <feGaussianBlur stdDeviation="2.5" result="coloredBlur"/>
                                <feMerge>
                                    <feMergeNode in="coloredBlur"/>
                                    <feMergeNode in="SourceGraphic"/>
                                </feMerge>
                             </filter>
                         </defs>
                         {/* Track Path */}
                         <path 
                           d="M 50,150 C 50,50 150,50 200,100 C 250,150 350,50 350,150 C 350,250 250,250 200,200 C 150,150 50,250 50,150 Z" 
                           fill="none" 
                           stroke="rgba(255,255,255,0.1)" 
                           strokeWidth="12" 
                           strokeLinecap="round"
                         />
                         <path 
                           d="M 50,150 C 50,50 150,50 200,100 C 250,150 350,50 350,150 C 350,250 250,250 200,200 C 150,150 50,250 50,150 Z" 
                           fill="none" 
                           stroke="url(#trackGradient)" 
                           strokeWidth="3" 
                           strokeLinecap="round"
                           filter="url(#glow)"
                           className="animate-[dash_5s_linear_infinite]"
                           strokeDasharray="1000"
                           strokeDashoffset="1000"
                         />
                         {/* Player Marker */}
                         <circle cx="50" cy="150" r="4" fill="white" className="animate-[moveMarker_5s_linear_infinite]" />
                      </svg>
                  </div>

                  {/* Overlays */}
                  <div className="absolute top-6 left-6 z-10 w-48">
                      <div className="bg-slate-900/80 backdrop-blur-md border border-orange-500/50 rounded-xl p-4 shadow-lg">
                          <div className="text-slate-400 text-xs font-bold uppercase mb-1">Current Speed</div>
                          <div className="flex items-baseline">
                              <span className="text-3xl font-black text-orange-500 font-mono">112</span>
                              <span className="ml-1 text-sm text-slate-500 font-bold">km/h</span>
                          </div>
                      </div>
                  </div>
                </div>

                {/* Single Synced Chart */}
                <div className="bg-slate-800/40 backdrop-blur-sm border border-white/10 rounded-xl p-5 h-64">
                   <h3 className="text-sm font-bold text-slate-300 mb-4 flex items-center">
                       <TrendingUp className="w-4 h-4 mr-2" /> Speed Trace (Synced)
                   </h3>
                   <div className="h-48 w-full">
                      <TelemetryChart data={telemetryData} dataKey="speed" color="#3b82f6" unit="km/h" />
                   </div>
                </div>
              </>
            ) : (
              /* DATA MODE */
              <div className="space-y-6 animate-in fade-in duration-300">
                 <div className="grid grid-cols-1 md:grid-cols-3 gap-6">
                     {/* Mini Map Reference */}
                     <div className="bg-slate-800/40 backdrop-blur-sm border border-white/10 rounded-xl p-4 relative flex items-center justify-center overflow-hidden min-h-[200px]">
                        <h4 className="absolute top-3 left-3 text-xs font-bold text-slate-500 uppercase">Track Reference</h4>
                        <svg viewBox="0 0 400 300" className="w-full h-full opacity-60">
                           <path 
                             d="M 50,150 C 50,50 150,50 200,100 C 250,150 350,50 350,150 C 350,250 250,250 200,200 C 150,150 50,250 50,150 Z" 
                             fill="none" 
                             stroke="white" 
                             strokeWidth="4" 
                           />
                           <circle cx="200" cy="100" r="6" fill="#f97316" className="animate-pulse" />
                        </svg>
                     </div>

                     {/* Stats Grid */}
                     <div className="col-span-2 grid grid-cols-2 gap-4">
                        <StatCard label="Max Speed" value={currentLapData?.maxSpeed || '-'} unit="km/h" color="text-blue-400" />
                        <StatCard label="Max RPM" value={currentLapData?.maxRpm || '-'} unit="rpm" color="text-green-400" />
                        <StatCard label="Avg Temp" value={currentLapData?.waterTemp || '-'} unit="°C" color="text-red-400" />
                        <StatCard label="Lap Time" value={currentLapData?.time || '-'} unit="sec" color="text-orange-400" />
                     </div>
                 </div>

                 {/* Charts */}
                 <div className="bg-slate-800/40 backdrop-blur-sm border border-white/10 rounded-xl p-6">
                    <h3 className="text-sm font-bold text-slate-300 mb-4">Speed (km/h)</h3>
                    <div className="h-64">
                        <TelemetryChart data={telemetryData} dataKey="speed" color="#3b82f6" unit=" km/h" />
                    </div>
                 </div>

                 <div className="bg-slate-800/40 backdrop-blur-sm border border-white/10 rounded-xl p-6">
                    <h3 className="text-sm font-bold text-slate-300 mb-4">Engine RPM</h3>
                    <div className="h-64">
                        <TelemetryChart data={telemetryData} dataKey="rpm" color="#22c55e" unit=" rpm" />
                    </div>
                 </div>

                 <div className="grid md:grid-cols-2 gap-6">
                    <div className="bg-slate-800/40 backdrop-blur-sm border border-white/10 rounded-xl p-6">
                        <h3 className="text-sm font-bold text-slate-300 mb-4">Water Temp (°C)</h3>
                        <div className="h-48">
                            <TelemetryChart data={telemetryData} dataKey="waterTemp" color="#ef4444" unit="°C" />
                        </div>
                    </div>
                    <div className="bg-slate-800/40 backdrop-blur-sm border border-white/10 rounded-xl p-6">
                        <h3 className="text-sm font-bold text-slate-300 mb-4">Delta (s)</h3>
                        <div className="h-48">
                            <TelemetryChart data={telemetryData} dataKey="delta" color="#f97316" unit="s" />
                        </div>
                    </div>
                 </div>
              </div>
            )}
          </div>
        </div>
      </div>
      
      {/* Global styles for custom scrollbar if needed */}
      <style jsx global>{`
        .scrollbar-thin::-webkit-scrollbar {
          width: 6px;
        }
        .scrollbar-track-transparent::-webkit-scrollbar-track {
          background: transparent;
        }
        .scrollbar-thumb-slate-600::-webkit-scrollbar-thumb {
          background-color: #475569;
          border-radius: 20px;
        }
        @keyframes dash {
          to {
            stroke-dashoffset: 0;
          }
        }
      `}</style>
    </div>
  );
}

// Reusable Recharts Component
function TelemetryChart({ data, dataKey, color, unit }: any) {
  return (
    <ResponsiveContainer width="100%" height="100%">
      <AreaChart data={data} margin={{ top: 5, right: 0, left: 0, bottom: 0 }}>
        <defs>
          <linearGradient id={`gradient-${dataKey}`} x1="0" y1="0" x2="0" y2="1">
            <stop offset="5%" stopColor={color} stopOpacity={0.3}/>
            <stop offset="95%" stopColor={color} stopOpacity={0}/>
          </linearGradient>
        </defs>
        <CartesianGrid strokeDasharray="3 3" stroke="rgba(255,255,255,0.05)" vertical={false} />
        <XAxis 
            dataKey="distance" 
            hide 
            // tick={{fill: '#64748b', fontSize: 10}} 
            // tickLine={false}
            // axisLine={{ stroke: '#334155' }}
        />
        <YAxis 
            tick={{fill: '#64748b', fontSize: 10}} 
            tickLine={false} 
            axisLine={false}
            width={30}
            domain={['auto', 'auto']}
        />
        <Tooltip
           contentStyle={{ backgroundColor: '#1e293b', borderColor: '#334155', color: '#f1f5f9' }}
           itemStyle={{ color: '#f1f5f9' }}
           wrapperStyle={{ outline: 'none' }}
           labelStyle={{ display: 'none' }}
           formatter={(value: any) => [`${value}${unit}`, '']}
        />
        <Area 
            type="monotone" 
            dataKey={dataKey} 
            stroke={color} 
            fillOpacity={1} 
            fill={`url(#gradient-${dataKey})`} 
            strokeWidth={2}
            isAnimationActive={true}
        />
      </AreaChart>
    </ResponsiveContainer>
  );
}

function StatCard({ label, value, unit, color }: any) {
    return (
        <div className="bg-slate-800/40 backdrop-blur-sm border border-white/10 rounded-xl p-4 flex flex-col justify-center">
            <div className="text-slate-500 text-xs font-bold uppercase mb-1">{label}</div>
            <div className={`text-2xl font-black font-mono ${color}`}>
                {value} <span className="text-sm text-slate-500 font-bold ml-1">{unit}</span>
            </div>
        </div>
    )
}
