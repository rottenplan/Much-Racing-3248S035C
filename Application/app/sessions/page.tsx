import Link from 'next/link';
import { Clock, MapPin, Calendar, TrendingUp, Gauge, Activity, Zap } from 'lucide-react';
import fs from 'fs';
import path from 'path';


// Helper to get sessions from disk
async function getSessions() {
  const sessionsDir = path.join(process.cwd(), 'data', 'sessions');
  if (!fs.existsSync(sessionsDir)) {
    return [];
  }

  const files = fs.readdirSync(sessionsDir).filter(file => file.endsWith('.json'));
  const sessions = files.map(file => {
    try {
      const content = fs.readFileSync(path.join(sessionsDir, file), 'utf-8');
      return JSON.parse(content);
    } catch (e) {
      console.error(`Error reading session file ${file}`, e);
      return null;
    }
  }).filter(s => s !== null);

  // Sort by date desc
  return sessions.sort((a, b) => new Date(b.uploadDate).getTime() - new Date(a.uploadDate).getTime());
}

export default async function SessionsPage() {
  const sessions = await getSessions();

  return (
    <div className="min-h-screen bg-background text-foreground pb-24">
      {/* Header with Carbon Fiber */}


      {/* Content */}
      <div className="container mx-auto px-4 py-6">
        <div className="flex flex-col md:flex-row md:items-center md:justify-between mb-6">
          <div>
            <p className="text-text-secondary text-sm">View and analyze your sessions</p>
          </div>
          <Link href="/device" className="mt-3 md:mt-0 bg-primary hover:bg-primary-hover text-white px-4 py-2 rounded-lg transition font-racing text-sm">
            UPLOAD SESSION
          </Link>
        </div>

        {/* Filter Bar */}
        <div className="carbon-bg border border-border-color rounded-xl p-3 mb-6 flex flex-wrap gap-2">
          <button className="bg-primary text-white px-3 py-1.5 rounded text-xs font-racing">
            ALL TRACKS
          </button>
          <button className="bg-background-secondary hover:bg-card-bg text-text-secondary hover:text-white px-3 py-1.5 rounded text-xs font-racing transition border border-border-color">
            SENTUL KARTING
          </button>
          <button className="bg-background-secondary hover:bg-card-bg text-text-secondary hover:text-white px-3 py-1.5 rounded text-xs font-racing transition border border-border-color">
            MANDALIKA
          </button>
          <div className="w-px h-6 bg-border-color mx-2 hidden md:block"></div>
          <button className="bg-background-secondary hover:bg-card-bg text-text-secondary hover:text-white px-3 py-1.5 rounded text-xs font-racing transition border border-border-color flex items-center gap-2">
            <Calendar className="w-3 h-3" /> DATE RANGE
          </button>
          <button className="bg-background-secondary hover:bg-card-bg text-text-secondary hover:text-white px-3 py-1.5 rounded text-xs font-racing transition border border-border-color flex items-center gap-2">
            <Gauge className="w-3 h-3" /> VEHICLE
          </button>
        </div>

        {/* Sessions List */}
        <div className="space-y-4">
          {sessions.length === 0 ? (
            <div className="text-center py-20 carbon-bg rounded-xl border border-dashed border-border-color">
              <p className="text-text-secondary text-base font-racing">NO SESSIONS FOUND</p>
              <p className="text-text-secondary text-sm mt-2">Sync your device to see your runs here.</p>
            </div>
          ) : (
            sessions.map((session: any) => (
              <SessionCard key={session.id} session={session} />
            ))
          )}
        </div>
      </div>

    </div>
  );
}

function SessionCard({ session }: { session: any }) {
  const dateObj = new Date(session.uploadDate);
  const dateStr = dateObj.toLocaleDateString();
  const timeStr = dateObj.toLocaleTimeString([], { hour: '2-digit', minute: '2-digit' });

  // Calculate Duration roughly (last point time - first point time)
  let duration = "00:00";
  if (session.points && session.points.length > 0) {
    const start = parseFloat(session.points[0].time);
    const end = parseFloat(session.points[session.points.length - 1].time);
    const diffSeconds = (end - start) / 1000;
    const mins = Math.floor(diffSeconds / 60);
    const secs = Math.floor(diffSeconds % 60);
    duration = `${mins}:${secs.toString().padStart(2, '0')}`;
  }

  return (
    <Link href={`/sessions/${session.id}`}>
      <div className="carbon-bg border border-border-color rounded-xl p-4 hover:border-primary/50 transition group">
        <div className="flex flex-col md:flex-row md:items-center md:justify-between mb-3">
          <div>
            <h3 className="text-foreground text-base font-racing mb-2 flex items-center group-hover:text-primary transition">
              <MapPin className="w-4 h-4 mr-2 text-primary" />
              {session.originalFilename || "Unknown Session"}
            </h3>
            <div className="flex items-center gap-3 text-xs text-text-secondary">
              <span className="flex items-center gap-1">
                <Calendar className="w-3 h-3" />
                {dateStr}
              </span>
              <span className="flex items-center gap-1">
                <Clock className="w-3 h-3" />
                {timeStr}
              </span>
            </div>
          </div>
          <div className="mt-3 md:mt-0 text-right">
            <div className="text-text-secondary text-xs">Session ID</div>
            <div className="text-foreground font-data text-xs">{session.id}</div>
          </div>
        </div>

        <div className="grid grid-cols-2 md:grid-cols-4 gap-3 pt-3 border-t border-border-color">
          <div>
            <div className="text-text-secondary text-xs">Data Points</div>
            <div className="text-foreground font-data font-bold text-sm">{session.stats?.totalPoints || 0}</div>
          </div>
          <div>
            <div className="text-text-secondary text-xs">Top Speed</div>
            <div className="text-foreground font-data font-bold text-sm flex items-center gap-1">
              <Gauge className="w-3 h-3 text-primary" />
              {session.stats?.maxSpeed?.toFixed(1) || 0} km/h
            </div>
          </div>
          <div>
            <div className="text-text-secondary text-xs">Max RPM</div>
            <div className="text-foreground font-data font-bold text-sm flex items-center gap-1">
              <Activity className="w-3 h-3 text-warning" />
              {session.stats?.maxRpm?.toFixed(0) || 0}
            </div>
          </div>
          <div>
            <div className="text-text-secondary text-xs">Duration</div>
            <div className="text-highlight font-data font-bold text-sm flex items-center gap-1">
              <Clock className="w-3 h-3" />
              {duration}
            </div>
          </div>
        </div>
      </div>
    </Link>
  );
}
