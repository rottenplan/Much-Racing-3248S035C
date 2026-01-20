'use client';

import { use } from 'react';
import Link from 'next/link';
import { ChevronLeft, Share2, Download, Clock, Zap, MapPin, Gauge, Activity } from 'lucide-react';
import BottomNav from '../../components/BottomNav';
import {
  LineChart, Line, XAxis, YAxis, CartesianGrid, Tooltip, ResponsiveContainer, ReferenceLine
} from 'recharts';
import SessionMapWrapper from '../../components/SessionMapWrapper';

// Mock Coordinates for visual path (Sentulish shape)
const MOCK_PATH = Array.from({ length: 100 }, (_, i) => {
  const angle = (i / 100) * Math.PI * 2;
  return {
    lat: -6.535 + Math.cos(angle) * 0.003 + Math.sin(angle * 3) * 0.001,
    lng: 106.858 + Math.sin(angle) * 0.003 + Math.cos(angle * 2) * 0.001
  };
});

// Mock Data for a single session
const SESSION_DATA = {
  id: 1,
  track: 'Sentul Karting Circuit',
  date: '2026-01-06',
  time: '14:30',
  vehicle: 'Vespa Tune Up',
  bestLap: '48.234',
  totalLaps: 25,
  maxSpeed: '112 km/h',
  avgSpeed: '85 km/h',
  laps: [
    { lap: 1, time: '50.123', s1: '18.2', s2: '15.5', s3: '16.4', diff: '+1.889' },
    { lap: 2, time: '49.500', s1: '18.0', s2: '15.2', s3: '16.3', diff: '+1.266' },
    { lap: 3, time: '48.500', s1: '17.8', s2: '15.0', s3: '15.7', diff: '+0.266' },
    { lap: 4, time: '48.234', s1: '17.7', s2: '14.9', s3: '15.6', diff: '-0.000', isBest: true },
    { lap: 5, time: '48.800', s1: '17.9', s2: '15.1', s3: '15.8', diff: '+0.566' },
    { lap: 6, time: '49.100', s1: '18.1', s2: '15.2', s3: '15.8', diff: '+0.866' },
  ],
  // Mock Telemetry Points
  points: Array.from({ length: 100 }, (_, i) => ({
    time: i * 0.5,
    speed: 60 + Math.random() * 50 + Math.sin(i / 10) * 20,
    rpm: 8000 + Math.random() * 2000 + Math.sin(i / 10) * 1000,
    delta: Math.sin(i / 20) * 0.5,
  }))
};

export default function SessionDetailPage({ params }: { params: Promise<{ id: string }> }) {
  const resolvedParams = use(params);

  return (
    <div className="min-h-screen bg-background text-foreground pb-24">
      {/* Header */}
      <header className="carbon-bg backdrop-blur-md border-b border-border-color sticky top-0 z-20">
        <div className="container mx-auto px-4 py-4 flex items-center justify-between">
          {/* ... header content ... */}
          <div className="flex items-center gap-2">
            <Link href="/dashboard" className="p-2 -ml-2 hover:bg-white/5 rounded-full transition">
              <ChevronLeft className="w-6 h-6 text-foreground" />
            </Link>
            <div>
              <h1 className="text-lg font-racing tracking-wide">{SESSION_DATA.track}</h1>
              <p className="text-text-secondary text-xs font-medium">{SESSION_DATA.date} • {SESSION_DATA.time}</p>
            </div>
          </div>

          <button className="p-2 -mr-2 hover:bg-white/5 rounded-full transition">
            <Share2 className="w-5 h-5 text-primary" />
          </button>
        </div>
      </header>

      <div className="container mx-auto px-4 py-6 space-y-6">

        {/* Trajectory Map Section */}
        <div className="carbon-bg border border-border-color rounded-xl h-[300px] relative overflow-hidden shadow-lg">
          <SessionMapWrapper points={MOCK_PATH} />
        </div>

        {/* Key Metrics */}
        <div className="grid grid-cols-2 md:grid-cols-4 gap-4">
          <div className="carbon-bg border border-border-color rounded-xl p-4 text-center">
            <div className="text-text-secondary text-xs uppercase tracking-wider mb-1">Best Lap</div>
            <div className="text-3xl font-data font-bold text-highlight">{SESSION_DATA.bestLap}</div>
          </div>
          <div className="carbon-bg border border-border-color rounded-xl p-4 text-center">
            <div className="text-text-secondary text-xs uppercase tracking-wider mb-1">Max Speed</div>
            <div className="text-xl font-data font-bold text-foreground">{SESSION_DATA.maxSpeed}</div>
          </div>
          <div className="carbon-bg border border-border-color rounded-xl p-4 text-center">
            <div className="text-text-secondary text-xs uppercase tracking-wider mb-1">Avg Speed</div>
            <div className="text-xl font-data font-bold text-foreground">{SESSION_DATA.avgSpeed}</div>
          </div>
          <div className="carbon-bg border border-border-color rounded-xl p-4 text-center">
            <div className="text-text-secondary text-xs uppercase tracking-wider mb-1">Vehicle</div>
            <div className="text-sm font-racing text-primary truncate">{SESSION_DATA.vehicle}</div>
          </div>
        </div>

        {/* Analysis Charts */}
        <div className="carbon-bg border border-border-color rounded-xl p-4 space-y-4">
          <div className="flex items-center gap-2 mb-2">
            <Activity className="w-5 h-5 text-primary" />
            <h2 className="font-racing text-lg text-foreground">TELEMETRY ANALYSIS</h2>
          </div>

          {/* Speed Chart */}
          <ChartBlock title="Speed (km/h)" height={180}>
            <LineChart data={SESSION_DATA.points}>
              <CartesianGrid strokeDasharray="3 3" stroke="#333" vertical={false} />
              <YAxis stroke="#666" fontSize={10} tickCount={5} domain={[0, 'auto']} />
              <Tooltip content={<CustomTooltip />} />
              <Line type="monotone" dataKey="speed" stroke="#00ccff" strokeWidth={2} dot={false} activeDot={{ r: 4, fill: '#fff' }} />
            </LineChart>
          </ChartBlock>

          {/* RPM Chart */}
          <ChartBlock title="RPM" height={120}>
            <LineChart data={SESSION_DATA.points}>
              <CartesianGrid strokeDasharray="3 3" stroke="#333" vertical={false} />
              <YAxis stroke="#666" fontSize={10} tickCount={3} domain={[0, 'auto']} />
              <Tooltip content={<CustomTooltip />} />
              <Line type="monotone" dataKey="rpm" stroke="#ff0000" strokeWidth={1.5} dot={false} />
            </LineChart>
          </ChartBlock>

          {/* Delta Chart */}
          <ChartBlock title="Delta (s)" height={120}>
            <LineChart data={SESSION_DATA.points}>
              <CartesianGrid strokeDasharray="3 3" stroke="#333" vertical={false} />
              <YAxis stroke="#666" fontSize={10} tickCount={3} />
              <ReferenceLine y={0} stroke="#666" />
              <Tooltip content={<CustomTooltip />} />
              <Line type="monotone" dataKey="delta" stroke="#00ff00" strokeWidth={1.5} dot={false} />
            </LineChart>
          </ChartBlock>
        </div>

        {/* Lap List */}
        <div className="carbon-bg border border-border-color rounded-xl overflow-hidden">
          <div className="p-4 border-b border-border-color flex justify-between items-center">
            <h2 className="font-racing text-lg">LAP TIMES</h2>
            <button className="text-primary text-xs flex items-center gap-1 hover:underline">
              <Download className="w-3 h-3" /> EXPORT CSV
            </button>
          </div>

          <div className="overflow-x-auto">
            <table className="w-full text-sm font-data">
              <thead>
                <tr className="bg-background-secondary text-text-secondary text-xs uppercase text-left">
                  <th className="py-3 px-4">#</th>
                  <th className="py-3 px-4">Time</th>
                  <th className="py-3 px-4">S1</th>
                  <th className="py-3 px-4">S2</th>
                  <th className="py-3 px-4">S3</th>
                  <th className="py-3 px-4 text-right">Delta</th>
                </tr>
              </thead>
              <tbody>
                {SESSION_DATA.laps.map((lap) => (
                  <tr key={lap.lap} className={`border-b border-border-color/50 hover:bg-card-bg transition ${lap.isBest ? 'bg-highlight/5' : ''}`}>
                    <td className="py-3 px-4 text-text-secondary">{lap.lap}</td>
                    <td className={`py-3 px-4 font-bold ${lap.isBest ? 'text-highlight' : 'text-foreground'}`}>
                      {lap.time}
                      {lap.isBest && <span className="ml-2 text-[10px] bg-highlight/20 text-highlight px-1.5 py-0.5 rounded">PB</span>}
                    </td>
                    <td className="py-3 px-4 text-text-secondary">{lap.s1}</td>
                    <td className="py-3 px-4 text-text-secondary">{lap.s2}</td>
                    <td className="py-3 px-4 text-text-secondary">{lap.s3}</td>
                    <td className={`py-3 px-4 text-right ${lap.diff.startsWith('-') ? 'text-highlight' : 'text-warning'}`}>
                      {lap.diff}
                    </td>
                  </tr>
                ))}
              </tbody>
            </table>
          </div>
        </div>

      </div>
      <BottomNav />
    </div>
  );
}

function ChartBlock({ children, height, title }: { children: React.ReactNode, height: number, title?: string }) {
  return (
    <div className="relative w-full bg-background-secondary rounded-lg overflow-hidden border border-border-color" style={{ height }}>
      {title && <div className="absolute top-2 left-2 text-[10px] text-text-secondary font-bold z-10">{title}</div>}
      <ResponsiveContainer width="100%" height="100%">
        {children as any}
      </ResponsiveContainer>
    </div>
  )
}

const CustomTooltip = ({ active, payload, label }: any) => {
  if (active && payload && payload.length) {
    return (
      <div className="bg-card-bg border border-border-color p-2 text-xs text-foreground shadow-xl rounded">
        <p className="font-mono text-text-secondary mb-1">{`Time: ${Number(label).toFixed(1)}s`}</p>
        {payload.map((p: any) => (
          <div key={p.name} className="flex items-center gap-2">
            <div className="w-2 h-2 rounded-full" style={{ backgroundColor: p.color }}></div>
            <span className="font-bold">
              {p.name}: {Number(p.value).toFixed(1)}
            </span>
          </div>
        ))}
      </div>
    );
  }
  return null;
};
