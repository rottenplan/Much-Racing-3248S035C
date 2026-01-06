'use client';

import Link from 'next/link';
import { useState, useRef, useEffect } from 'react';
import { ChevronLeft, ChevronRight, MapPin, Trash2, RotateCcw, Save } from 'lucide-react';

interface TrackPoint {
  x: number;
  y: number;
  lat: number;
  lng: number;
}

export default function CreateTrack() {
  const [sidebarVisible, setSidebarVisible] = useState(true);
  const [trackPoints, setTrackPoints] = useState<TrackPoint[]>([]);
  const [country, setCountry] = useState('');
  const [address, setAddress] = useState('');
  const [trackType, setTrackType] = useState('Race track');
  const [trackName, setTrackName] = useState('');
  const [phone, setPhone] = useState('');
  const [postCode, setPostCode] = useState('');
  const [city, setCity] = useState('');
  const [uid, setUid] = useState('');
  const [startLineWidth, setStartLineWidth] = useState(12);
  const [startLineBearing, setStartLineBearing] = useState(0);
  const canvasRef = useRef<HTMLCanvasElement>(null);

  // Generate UID on mount
  useEffect(() => {
    const generateUID = () => {
      return 'xxxxxxxx-xxxx-4xxx-yxxx-xxxxxxxxxxxx'.replace(/[xy]/g, (c) => {
        const r = Math.random() * 16 | 0;
        const v = c === 'x' ? r : (r & 0x3 | 0x8);
        return v.toString(16);
      });
    };
    setUid(generateUID());
  }, []);

  // Draw map and track points
  useEffect(() => {
    const canvas = canvasRef.current;
    if (!canvas) return;

    const ctx = canvas.getContext('2d');
    if (!ctx) return;

    // Clear canvas
    ctx.fillStyle = '#1e293b';
    ctx.fillRect(0, 0, canvas.width, canvas.height);

    // Draw grid pattern (satellite-style)
    ctx.strokeStyle = '#334155';
    ctx.lineWidth = 1;
    for (let i = 0; i < canvas.width; i += 50) {
      ctx.beginPath();
      ctx.moveTo(i, 0);
      ctx.lineTo(i, canvas.height);
      ctx.stroke();
    }
    for (let i = 0; i < canvas.height; i += 50) {
      ctx.beginPath();
      ctx.moveTo(0, i);
      ctx.lineTo(canvas.width, i);
      ctx.stroke();
    }

    // Draw track points and lines
    if (trackPoints.length > 0) {
      // Draw lines connecting points
      ctx.strokeStyle = '#f97316';
      ctx.lineWidth = 3;
      ctx.beginPath();
      ctx.moveTo(trackPoints[0].x, trackPoints[0].y);
      for (let i = 1; i < trackPoints.length; i++) {
        ctx.lineTo(trackPoints[i].x, trackPoints[i].y);
      }
      ctx.stroke();

      // Draw points
      trackPoints.forEach((point, index) => {
        ctx.fillStyle = index === 0 ? '#22c55e' : '#f97316';
        ctx.beginPath();
        ctx.arc(point.x, point.y, 6, 0, Math.PI * 2);
        ctx.fill();

        // Draw point number
        ctx.fillStyle = '#ffffff';
        ctx.font = '12px sans-serif';
        ctx.fillText((index + 1).toString(), point.x + 10, point.y - 10);
      });
    }

    // Draw placeholder text
    if (trackPoints.length === 0) {
      ctx.fillStyle = '#64748b';
      ctx.font = '16px sans-serif';
      ctx.textAlign = 'center';
      ctx.fillText('Click on the map to add track points', canvas.width / 2, canvas.height / 2);
    }
  }, [trackPoints]);

  const handleCanvasClick = (e: React.MouseEvent<HTMLCanvasElement>) => {
    const canvas = canvasRef.current;
    if (!canvas) return;

    const rect = canvas.getBoundingClientRect();
    const x = e.clientX - rect.left;
    const y = e.clientY - rect.top;

    // Mock lat/lng conversion (in production, use real map coordinates)
    const lat = 0 + (y / canvas.height) * 0.01;
    const lng = 0 + (x / canvas.width) * 0.01;

    setTrackPoints([...trackPoints, { x, y, lat, lng }]);
  };

  const handleRemoveLastPoint = () => {
    if (trackPoints.length > 0) {
      setTrackPoints(trackPoints.slice(0, -1));
    }
  };

  const handleNewTrack = () => {
    if (confirm('Are you sure you want to clear all track data?')) {
      setTrackPoints([]);
      setCountry('');
      setAddress('');
      setTrackType('Race track');
      setTrackName('');
      setPhone('');
      setPostCode('');
      setCity('');
      setStartLineWidth(12);
      setStartLineBearing(0);
    }
  };

  const handleSaveTrack = () => {
    if (!trackName || trackPoints.length < 3) {
      alert('Please enter a track name and add at least 3 points on the map.');
      return;
    }

    const trackData = {
      uid,
      country,
      address,
      trackType,
      trackName,
      phone,
      postCode,
      city,
      startLineWidth,
      startLineBearing,
      points: trackPoints,
      createdAt: new Date().toISOString(),
    };

    // Save to localStorage (in production, send to API)
    const savedTracks = JSON.parse(localStorage.getItem('muchracing_tracks') || '[]');
    savedTracks.push(trackData);
    localStorage.setItem('muchracing_tracks', JSON.stringify(savedTracks));

    alert('Track saved successfully!');
    // Optionally redirect to tracks list
    // window.location.href = '/tracks';
  };

  const countries = [
    'Select a country',
    'Indonesia',
    'United States',
    'United Kingdom',
    'Germany',
    'France',
    'Italy',
    'Spain',
    'Japan',
    'Australia',
    'Brazil',
    'Canada',
    'Netherlands',
    'Belgium',
    'Austria',
    'Switzerland',
    'Singapore',
    'Malaysia',
    'Thailand',
  ];

  const trackTypes = [
    'Race track',
    'Street circuit',
    'Karting',
    'Oval',
    'Rally stage',
    'Drag strip',
    'Hillclimb',
  ];

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
              <Link href="/dashboard" className="text-slate-300 hover:text-white transition">Dashboard</Link>
              <Link href="/tracks" className="text-white font-semibold">Tracks</Link>
              <Link href="/sessions" className="text-slate-300 hover:text-white transition">Sessions</Link>
              <Link href="/device" className="text-slate-300 hover:text-white transition">Device</Link>
            </div>
            <Link href="/login" className="bg-orange-500 hover:bg-orange-600 text-white px-6 py-2 rounded-lg transition">
              Login
            </Link>
          </div>
        </div>
      </nav>

      {/* Main Content - Split Screen */}
      <div className="flex h-[calc(100vh-73px)]">
        {/* Sidebar */}
        <div
          className={`bg-slate-800/50 backdrop-blur-sm border-r border-slate-700 transition-all duration-300 overflow-y-auto ${
            sidebarVisible ? 'w-full md:w-96' : 'w-0'
          }`}
        >
          {sidebarVisible && (
            <div className="p-6 space-y-6">
              <div className="flex items-center justify-between">
                <h1 className="text-2xl font-bold text-white">Create Track</h1>
                <button
                  onClick={() => setSidebarVisible(false)}
                  className="md:hidden text-slate-400 hover:text-white"
                >
                  <ChevronLeft className="w-6 h-6" />
                </button>
              </div>

              {/* Country */}
              <div>
                <label className="block text-sm font-medium text-slate-300 mb-2">
                  Select a country *
                </label>
                <select
                  value={country}
                  onChange={(e) => setCountry(e.target.value)}
                  className="w-full bg-slate-700 text-white border border-slate-600 rounded-lg px-4 py-2 focus:outline-none focus:ring-2 focus:ring-orange-500"
                >
                  {countries.map((c) => (
                    <option key={c} value={c}>
                      {c}
                    </option>
                  ))}
                </select>
              </div>

              {/* Address */}
              <div>
                <label className="block text-sm font-medium text-slate-300 mb-2">
                  Address
                </label>
                <div className="flex gap-2">
                  <input
                    type="text"
                    value={address}
                    onChange={(e) => setAddress(e.target.value)}
                    className="flex-1 bg-slate-700 text-white border border-slate-600 rounded-lg px-4 py-2 focus:outline-none focus:ring-2 focus:ring-orange-500"
                    placeholder="Enter address"
                  />
                  <button className="bg-green-600 hover:bg-green-700 text-white px-4 py-2 rounded-lg transition">
                    <MapPin className="w-5 h-5" />
                  </button>
                </div>
              </div>

              {/* Track Type */}
              <div>
                <label className="block text-sm font-medium text-slate-300 mb-2">
                  Track type *
                </label>
                <select
                  value={trackType}
                  onChange={(e) => setTrackType(e.target.value)}
                  className="w-full bg-slate-700 text-white border border-slate-600 rounded-lg px-4 py-2 focus:outline-none focus:ring-2 focus:ring-orange-500"
                >
                  {trackTypes.map((type) => (
                    <option key={type} value={type}>
                      {type}
                    </option>
                  ))}
                </select>
              </div>

              {/* Track Name */}
              <div>
                <label className="block text-sm font-medium text-slate-300 mb-2">
                  Track name *
                </label>
                <input
                  type="text"
                  value={trackName}
                  onChange={(e) => setTrackName(e.target.value)}
                  className="w-full bg-slate-700 text-white border border-slate-600 rounded-lg px-4 py-2 focus:outline-none focus:ring-2 focus:ring-orange-500"
                  placeholder="Enter track name"
                />
              </div>

              {/* Contact Info */}
              <div className="grid grid-cols-3 gap-4">
                <div>
                  <label className="block text-sm font-medium text-slate-300 mb-2">
                    Phone
                  </label>
                  <input
                    type="tel"
                    value={phone}
                    onChange={(e) => setPhone(e.target.value)}
                    className="w-full bg-slate-700 text-white border border-slate-600 rounded-lg px-4 py-2 focus:outline-none focus:ring-2 focus:ring-orange-500"
                  />
                </div>
                <div>
                  <label className="block text-sm font-medium text-slate-300 mb-2">
                    Post code
                  </label>
                  <input
                    type="text"
                    value={postCode}
                    onChange={(e) => setPostCode(e.target.value)}
                    className="w-full bg-slate-700 text-white border border-slate-600 rounded-lg px-4 py-2 focus:outline-none focus:ring-2 focus:ring-orange-500"
                  />
                </div>
                <div>
                  <label className="block text-sm font-medium text-slate-300 mb-2">
                    City
                  </label>
                  <input
                    type="text"
                    value={city}
                    onChange={(e) => setCity(e.target.value)}
                    className="w-full bg-slate-700 text-white border border-slate-600 rounded-lg px-4 py-2 focus:outline-none focus:ring-2 focus:ring-orange-500"
                  />
                </div>
              </div>

              {/* UID */}
              <div>
                <label className="block text-sm font-medium text-slate-300 mb-2">
                  UID
                </label>
                <input
                  type="text"
                  value={uid}
                  readOnly
                  className="w-full bg-slate-900 text-slate-400 border border-slate-600 rounded-lg px-4 py-2 cursor-not-allowed"
                />
              </div>

              {/* Start Line Calibration */}
              <div className="border-t border-slate-700 pt-6">
                <h3 className="text-lg font-semibold text-white mb-4">Start Line Calibration</h3>
                <div className="grid grid-cols-2 gap-4">
                  <div>
                    <label className="block text-sm font-medium text-slate-300 mb-2">
                      Width (meters)
                    </label>
                    <input
                      type="number"
                      value={startLineWidth}
                      onChange={(e) => setStartLineWidth(Number(e.target.value))}
                      className="w-full bg-slate-700 text-white border border-slate-600 rounded-lg px-4 py-2 focus:outline-none focus:ring-2 focus:ring-orange-500"
                    />
                  </div>
                  <div>
                    <label className="block text-sm font-medium text-slate-300 mb-2">
                      Bearing (degree)
                    </label>
                    <input
                      type="number"
                      value={startLineBearing}
                      onChange={(e) => setStartLineBearing(Number(e.target.value))}
                      className="w-full bg-slate-700 text-white border border-slate-600 rounded-lg px-4 py-2 focus:outline-none focus:ring-2 focus:ring-orange-500"
                    />
                  </div>
                </div>
              </div>

              {/* Action Buttons */}
              <div className="space-y-3 border-t border-slate-700 pt-6">
                <button
                  onClick={handleRemoveLastPoint}
                  disabled={trackPoints.length === 0}
                  className="w-full bg-red-600 hover:bg-red-700 disabled:bg-slate-700 disabled:cursor-not-allowed text-white px-6 py-3 rounded-lg transition flex items-center justify-center gap-2"
                >
                  <Trash2 className="w-5 h-5" />
                  Remove Last Point
                </button>
                <button
                  onClick={handleNewTrack}
                  className="w-full bg-orange-600 hover:bg-orange-700 text-white px-6 py-3 rounded-lg transition flex items-center justify-center gap-2"
                >
                  <RotateCcw className="w-5 h-5" />
                  New Track
                </button>
                <button
                  onClick={handleSaveTrack}
                  className="w-full bg-green-600 hover:bg-green-700 text-white px-6 py-3 rounded-lg transition flex items-center justify-center gap-2"
                >
                  <Save className="w-5 h-5" />
                  Save Track
                </button>
              </div>

              {/* Track Info */}
              {trackPoints.length > 0 && (
                <div className="bg-slate-700/50 rounded-lg p-4">
                  <p className="text-slate-300 text-sm">
                    <span className="font-semibold">Points:</span> {trackPoints.length}
                  </p>
                </div>
              )}
            </div>
          )}
        </div>

        {/* Map Area */}
        <div className="flex-1 relative">
          {/* Toggle Button */}
          {!sidebarVisible && (
            <button
              onClick={() => setSidebarVisible(true)}
              className="absolute top-4 left-4 z-10 bg-slate-800 hover:bg-slate-700 text-white p-2 rounded-lg transition"
            >
              <ChevronRight className="w-6 h-6" />
            </button>
          )}

          {/* Canvas */}
          <canvas
            ref={canvasRef}
            width={1200}
            height={800}
            onClick={handleCanvasClick}
            className="w-full h-full cursor-crosshair"
          />
        </div>
      </div>
    </div>
  );
}
