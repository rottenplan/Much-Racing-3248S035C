'use client';

import Link from 'next/link';
import { ArrowLeft, MapPin, Clock, TrendingUp, Play, Pause, Maximize, Download, Map, BarChart3 } from 'lucide-react';
import { useParams } from 'next/navigation';
import { useState } from 'react';

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

export default function SessionDetailPage() {
  const params = useParams();
  const sessionId = params.id as string;
  const session = sessionData[sessionId];
  const [selectedLaps, setSelectedLaps] = useState<number[]>([3]); // Best lap selected by default
  const [isPlaying, setIsPlaying] = useState(false);
  const [viewMode, setViewMode] = useState<'map' | 'data'>('map'); // Toggle between MAP and DATA mode

  if (!session) {
    return (
      <div className="min-h-screen bg-gradient-to-br from-slate-900 via-blue-900 to-slate-900 flex items-center justify-center">
        <div className="text-white text-2xl">Session not found</div>
      </div>
    );
  }

  const toggleLapSelection = (lapNum: number) => {
    setSelectedLaps(prev =>
      prev.includes(lapNum)
        ? prev.filter(n => n !== lapNum)
        : [...prev, lapNum]
    );
  };

  return (
    <div className="min-h-screen bg-gradient-to-br from-slate-900 via-blue-900 to-slate-900">
      {/* Navigation */}
      <nav className="bg-slate-900/50 backdrop-blur-sm border-b border-slate-700">
        <div className="container mx-auto px-6 py-4">
          <div className="flex items-center justify-between">
            <Link href="/" className="flex items-center space-x-2">
              <div className="w-10 h-10 bg-gradient-to-br from-orange-500 to-red-600 rounded-lg flex items-center justify-center">
                <span className="text-white font-bold text-xl">R</span>
              </div>
              <span className="text-white text-2xl font-bold">Much Racing</span>
            </Link>
            <div className="hidden md:flex space-x-6">
              <Link href="/dashboard" className="text-white font-semibold">Dashboard</Link>
              <Link href="/tracks" className="text-slate-300 hover:text-white transition">Tracks</Link>
              <Link href="/sessions" className="text-slate-300 hover:text-white transition">Sessions</Link>
              <Link href="/device" className="text-slate-300 hover:text-white transition">Device</Link>
            </div>
          </div>
        </div>
      </nav>

      {/* Content */}
      <div className="container mx-auto px-6 py-12">
        {/* Back Button */}
        <Link href="/dashboard" className="inline-flex items-center text-slate-300 hover:text-white transition mb-6">
          <ArrowLeft className="w-5 h-5 mr-2" />
          Back to Dashboard
        </Link>

        {/* Session Header */}
        <div className="bg-slate-800/50 backdrop-blur-sm border border-slate-700 rounded-xl p-8 mb-8">
          <div className="flex flex-col md:flex-row md:items-start md:justify-between mb-6">
            <div>
              <h1 className="text-4xl font-bold text-white mb-2">{session.track}</h1>
              <p className="text-slate-400 text-lg flex items-center mb-2">
                <MapPin className="w-5 h-5 mr-2" />
                {session.location}
              </p>
              <p className="text-slate-400 flex items-center">
                <Clock className="w-5 h-5 mr-2" />
                {session.date} {session.time}
              </p>
            </div>
            <div className="mt-4 md:mt-0 text-right">
              <div className="text-5xl font-bold text-transparent bg-clip-text bg-gradient-to-r from-orange-400 to-red-600 mb-2">
                {session.bestLap}
              </div>
              <div className="text-slate-400">Best Lap Time</div>
            </div>
          </div>

          {/* Session Stats */}
          <div className="grid grid-cols-2 md:grid-cols-4 gap-4">
            <StatBox label="Total Laps" value={session.totalLaps.toString()} />
            <StatBox label="Avg Lap Time" value={session.avgLap} />
            <StatBox label="Total Time" value={session.totalTime} />
            <StatBox label="Category" value={session.category} />
          </div>
        </div>

        {/* Playback Controls */}
        <div className="bg-slate-800/50 backdrop-blur-sm border border-slate-700 rounded-xl p-4 mb-8">
          <div className="flex items-center justify-between">
            <div className="flex items-center space-x-4">
              <button
                onClick={() => setIsPlaying(!isPlaying)}
                className="bg-orange-500 hover:bg-orange-600 text-white p-3 rounded-lg transition"
              >
                {isPlaying ? <Pause className="w-5 h-5" /> : <Play className="w-5 h-5" />}
              </button>
              <span className="text-slate-400">Playback Controls</span>
            </div>
            <div className="flex items-center space-x-2">
              {/* View Mode Toggle */}
              <div className="flex bg-slate-900 rounded-lg p-1 mr-2">
                <button
                  onClick={() => setViewMode('map')}
                  className={`flex items-center space-x-2 px-4 py-2 rounded-lg transition ${
                    viewMode === 'map'
                      ? 'bg-orange-500 text-white'
                      : 'text-slate-400 hover:text-white'
                  }`}
                >
                  <Map className="w-4 h-4" />
                  <span className="text-sm font-semibold">MAP</span>
                </button>
                <button
                  onClick={() => setViewMode('data')}
                  className={`flex items-center space-x-2 px-4 py-2 rounded-lg transition ${
                    viewMode === 'data'
                      ? 'bg-orange-500 text-white'
                      : 'text-slate-400 hover:text-white'
                  }`}
                >
                  <BarChart3 className="w-4 h-4" />
                  <span className="text-sm font-semibold">DATA</span>
                </button>
              </div>
              <button className="bg-slate-700 hover:bg-slate-600 text-white p-2 rounded-lg transition">
                <Maximize className="w-5 h-5" />
              </button>
              <button className="bg-slate-700 hover:bg-slate-600 text-white p-2 rounded-lg transition">
                <Download className="w-5 h-5" />
              </button>
            </div>
          </div>
        </div>

        <div className="grid lg:grid-cols-4 gap-8">
          {/* Lap Selection Sidebar */}
          <div className="lg:col-span-1">
            <div className="bg-slate-800/50 backdrop-blur-sm border border-slate-700 rounded-xl p-6 sticky top-6">
              <h2 className="text-xl font-bold text-white mb-4">Lap Details</h2>
              <div className="space-y-2 max-h-96 overflow-y-auto">
                {session.laps.map((lap: any) => (
                  <div
                    key={lap.num}
                    onClick={() => toggleLapSelection(lap.num)}
                    className={`p-3 rounded-lg cursor-pointer transition ${
                      selectedLaps.includes(lap.num)
                        ? 'bg-orange-500/20 border border-orange-500'
                        : 'bg-slate-900/50 border border-slate-700 hover:border-slate-600'
                    }`}
                  >
                    <div className="flex items-center justify-between mb-1">
                      <span className="text-white font-semibold">Lap {lap.num}</span>
                      <span className={`font-mono text-sm ${
                        lap.num === 3 ? 'text-orange-400 font-bold' : 'text-slate-300'
                      }`}>
                        {lap.time}
                      </span>
                    </div>
                    <div className="text-slate-400 text-xs">
                      {lap.delta}
                    </div>
                  </div>
                ))}
              </div>
            </div>
          </div>

          {/* Content Area - MAP or DATA mode */}
          <div className="lg:col-span-3 space-y-6">
            {viewMode === 'map' ? (
              /* MAP MODE - Large Interactive Map */
              <>
                <div className="bg-slate-800/50 backdrop-blur-sm border border-slate-700 rounded-xl p-6">
                  <div className="flex items-center justify-between mb-4">
                    <h2 className="text-xl font-bold text-white">Track Map - Interactive View</h2>
                    <div className="flex items-center space-x-2">
                      <button className="bg-slate-700 hover:bg-slate-600 text-white px-3 py-1 rounded text-sm transition">
                        Satellite
                      </button>
                      <button className="bg-slate-700 hover:bg-slate-600 text-white px-3 py-1 rounded text-sm transition">
                        Dynamic
                      </button>
                    </div>
                  </div>
                  {/* Large Map */}
                  <div className="h-[600px] bg-gradient-to-br from-slate-700 to-slate-800 rounded-lg flex items-center justify-center border border-slate-600 relative overflow-hidden">
                    <div className="text-center">
                      <Map className="w-24 h-24 text-orange-500 mx-auto mb-4" />
                      <p className="text-slate-400 text-lg mb-2">Interactive Racing Line Visualization</p>
                      <p className="text-slate-500 text-sm">Map shows GPS track with speed overlay</p>
                      <p className="text-slate-500 text-sm mt-4">Integration: MapLibre GL / Leaflet.js</p>
                    </div>
                    {/* Speed indicator overlay */}
                    <div className="absolute top-4 left-4 bg-slate-900/90 border border-orange-500 rounded-lg p-3">
                      <div className="text-slate-400 text-xs mb-1">Current Speed</div>
                      <div className="text-orange-400 text-2xl font-bold">85.2 Km/h</div>
                    </div>
                  </div>
                  {/* Timeline Slider */}
                  <div className="mt-4">
                    <input
                      type="range"
                      min="0"
                      max="100"
                      className="w-full"
                    />
                    <div className="flex justify-between text-slate-400 text-sm mt-2">
                      <span>00:00.000</span>
                      <span>Lap 3 - Position: 45%</span>
                      <span>00:48.234</span>
                    </div>
                  </div>
                </div>

                {/* Single Speed Graph synced with map */}
                <TelemetryChart
                  title="Speed (Km/h) - Synced with Map"
                  color="blue"
                  data={session.laps}
                  dataKey="speed"
                  selectedLaps={selectedLaps}
                />
              </>
            ) : (
              /* DATA MODE - Multiple Telemetry Charts */
              <>
                {/* Track Map (smaller reference) */}
                <div className="bg-slate-800/50 backdrop-blur-sm border border-slate-700 rounded-xl p-6">
                  <h2 className="text-xl font-bold text-white mb-4">Track Map</h2>
                  <div className="h-64 bg-gradient-to-br from-slate-700 to-slate-800 rounded-lg flex items-center justify-center border border-slate-600">
                    <div className="text-center">
                      <MapPin className="w-16 h-16 text-orange-500 mx-auto mb-4" />
                      <p className="text-slate-400">Track reference map</p>
                    </div>
                  </div>
                </div>

                {/* Speed Chart */}
                <TelemetryChart
                  title="Speed (Km/h)"
                  color="blue"
                  data={session.laps}
                  dataKey="speed"
                  selectedLaps={selectedLaps}
                />

                {/* RPM Chart */}
                <TelemetryChart
                  title="RPM"
                  color="green"
                  data={session.laps}
                  dataKey="rpm"
                  selectedLaps={selectedLaps}
                />

                {/* Delta Chart */}
                <TelemetryChart
                  title="Delta (ms)"
                  color="orange"
                  data={session.laps}
                  dataKey="delta"
                  selectedLaps={selectedLaps}
                />

                {/* Temperature Charts */}
                <div className="grid md:grid-cols-2 gap-6">
                  <TelemetryChart
                    title="Water Temp (°C)"
                    color="red"
                    data={session.laps}
                    dataKey="waterTemp"
                    selectedLaps={selectedLaps}
                  />
                  <TelemetryChart
                    title="EGT (°C)"
                    color="purple"
                    data={session.laps}
                    dataKey="egt"
                    selectedLaps={selectedLaps}
                  />
                </div>
              </>
            )}
          </div>
        </div>
      </div>
    </div>
  );
}

function StatBox({ label, value }: { label: string; value: string }) {
  return (
    <div className="bg-slate-900/50 border border-slate-700 rounded-lg p-4">
      <div className="text-slate-500 text-sm mb-1">{label}</div>
      <div className="text-white text-xl font-bold">{value}</div>
    </div>
  );
}

function TelemetryChart({ title, color, data, dataKey, selectedLaps }: any) {
  const colorClasses: any = {
    blue: 'from-blue-500 to-cyan-600',
    green: 'from-green-500 to-emerald-600',
    orange: 'from-orange-500 to-red-600',
    red: 'from-red-500 to-pink-600',
    purple: 'from-purple-500 to-pink-600',
  };

  return (
    <div className="bg-slate-800/50 backdrop-blur-sm border border-slate-700 rounded-xl p-6">
      <h2 className="text-xl font-bold text-white mb-4">{title}</h2>
      <div className="h-48 bg-gradient-to-br from-slate-700 to-slate-800 rounded-lg flex items-center justify-center border border-slate-600 relative overflow-hidden">
        {/* Simple visualization placeholder */}
        <div className="absolute inset-0 flex items-end justify-around p-4">
          {data.map((lap: any, idx: number) => {
            const isSelected = selectedLaps.includes(lap.num);
            const value = typeof lap[dataKey] === 'string' ? parseFloat(lap[dataKey]) : lap[dataKey];
            const maxValue = Math.max(...data.map((l: any) => typeof l[dataKey] === 'string' ? parseFloat(l[dataKey]) : l[dataKey]));
            const height = (value / maxValue) * 100;
            
            return (
              <div key={idx} className="flex flex-col items-center flex-1">
                <div
                  className={`w-full rounded-t transition-all ${
                    isSelected
                      ? `bg-gradient-to-t ${colorClasses[color]} opacity-100`
                      : 'bg-slate-600 opacity-30'
                  }`}
                  style={{ height: `${height}%` }}
                ></div>
                <span className="text-slate-400 text-xs mt-2">{lap.num}</span>
              </div>
            );
          })}
        </div>
      </div>
      <div className="mt-4 text-slate-400 text-sm">
        Selected laps: {selectedLaps.join(', ')}
      </div>
    </div>
  );
}
