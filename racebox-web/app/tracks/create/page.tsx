'use client';

import Link from 'next/link';
import { useState, useRef, useEffect } from 'react';
import { ChevronLeft, ChevronRight, MapPin, Trash2, RotateCcw, Save } from 'lucide-react';
import TrackCreatorMapWrapper from '../../components/TrackCreatorMapWrapper';

interface TrackPoint {
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

  const handlePointAdd = (lat: number, lng: number) => {
    setTrackPoints([...trackPoints, { lat, lng }]);
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

  const handleSaveTrack = async () => {
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

    // Save to API
    try {
      const response = await fetch('/api/tracks/create', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify(trackData)
      });

      const result = await response.json();

      if (response.ok) {
        alert('Track saved to server successfully!');
        // clear form?
      } else {
        alert('Failed to save track: ' + result.error);
      }
    } catch (e) {
      console.error(e);
      alert('Network error saving track');
    }
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
              <div className="w-12 h-12 relative">
                <img
                  src="/logo.png"
                  alt="Much Racing Logo"
                  className="w-full h-full object-contain"
                />
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
          className={`bg-slate-800/50 backdrop-blur-sm border-r border-slate-700 transition-all duration-300 overflow-y-auto ${sidebarVisible ? 'w-full md:w-96' : 'w-0'
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
              className="absolute top-4 left-4 z-[999] bg-slate-800 hover:bg-slate-700 text-white p-2 rounded-lg transition shadow-lg"
            >
              <ChevronRight className="w-6 h-6" />
            </button>
          )}

          {/* Map Component */}
          <TrackCreatorMapWrapper
            points={trackPoints}
            onPointAdd={handlePointAdd}
          />
        </div>
      </div>
    </div>
  );
}
