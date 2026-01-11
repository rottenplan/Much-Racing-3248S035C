'use client';

import Link from 'next/link';
import { useState, useEffect } from 'react';
import { 
  Cpu, Plus, Trash2, Check, X, Edit2, Save, 
  RefreshCw, Monitor, Clock, Wifi
} from 'lucide-react';

interface Device {
  id: string;
  macAddress: string;
  name: string;
  activated: boolean;
  createdAt: string;
  lastSeen: string;
}

// Generate unique ID
const generateId = () => Math.random().toString(36).substring(2, 15);

// Format MAC address
const formatMac = (mac: string) => {
  const cleaned = mac.replace(/[^a-fA-F0-9]/g, '').toUpperCase();
  const formatted = cleaned.match(/.{1,2}/g)?.join(':') || cleaned;
  return formatted.substring(0, 17);
};

// Validate MAC address
const isValidMac = (mac: string) => {
  const pattern = /^([0-9A-Fa-f]{2}:){5}[0-9A-Fa-f]{2}$/;
  return pattern.test(mac);
};

export default function DevicesPage() {
  const [devices, setDevices] = useState<Device[]>([]);
  const [showAddModal, setShowAddModal] = useState(false);
  const [newMac, setNewMac] = useState('');
  const [newName, setNewName] = useState('');
  const [editingId, setEditingId] = useState<string | null>(null);
  const [editName, setEditName] = useState('');
  const [error, setError] = useState('');

  // Load devices from localStorage on mount
  useEffect(() => {
    const savedDevices = localStorage.getItem('racebox_devices');
    if (savedDevices) {
      setDevices(JSON.parse(savedDevices));
    }
  }, []);

  // Save devices to localStorage whenever they change
  useEffect(() => {
    if (devices.length > 0 || localStorage.getItem('racebox_devices')) {
      localStorage.setItem('racebox_devices', JSON.stringify(devices));
    }
  }, [devices]);

  // Add new device
  const handleAddDevice = () => {
    const formattedMac = formatMac(newMac);
    
    if (!isValidMac(formattedMac)) {
      setError('Invalid MAC address format. Use XX:XX:XX:XX:XX:XX');
      return;
    }

    // Check if MAC already exists
    if (devices.some(d => d.macAddress === formattedMac)) {
      setError('This MAC address is already registered');
      return;
    }

    const newDevice: Device = {
      id: generateId(),
      macAddress: formattedMac,
      name: newName || `Device ${devices.length + 1}`,
      activated: false,
      createdAt: new Date().toISOString(),
      lastSeen: '-'
    };

    setDevices([...devices, newDevice]);
    setShowAddModal(false);
    setNewMac('');
    setNewName('');
    setError('');
  };

  // Toggle device activation
  const toggleActivation = (id: string) => {
    setDevices(devices.map(d => 
      d.id === id ? { ...d, activated: !d.activated } : d
    ));
  };

  // Delete device
  const deleteDevice = (id: string) => {
    if (confirm('Are you sure you want to delete this device?')) {
      setDevices(devices.filter(d => d.id !== id));
    }
  };

  // Start editing device name
  const startEditing = (device: Device) => {
    setEditingId(device.id);
    setEditName(device.name);
  };

  // Save edited name
  const saveEdit = (id: string) => {
    setDevices(devices.map(d => 
      d.id === id ? { ...d, name: editName } : d
    ));
    setEditingId(null);
  };

  // Format date for display
  const formatDate = (dateStr: string) => {
    if (dateStr === '-') return '-';
    const date = new Date(dateStr);
    return date.toLocaleDateString('id-ID', { 
      day: '2-digit', 
      month: 'short', 
      year: 'numeric',
      hour: '2-digit',
      minute: '2-digit'
    });
  };

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
              <Link href="/tracks" className="text-slate-300 hover:text-white transition">Tracks</Link>
              <Link href="/sessions" className="text-slate-300 hover:text-white transition">Sessions</Link>
              <Link href="/device" className="text-slate-300 hover:text-white transition">Device</Link>
              <Link href="/devices" className="text-white font-semibold">Devices</Link>
            </div>
          </div>
        </div>
      </nav>

      {/* Content */}
      <div className="container mx-auto px-6 py-12">
        <div className="flex items-center justify-between mb-8">
          <div>
            <h1 className="text-4xl font-bold text-white mb-2">Device Management</h1>
            <p className="text-slate-400">Register and manage your Much Racing devices</p>
          </div>
          <button
            onClick={() => setShowAddModal(true)}
            className="bg-orange-500 hover:bg-orange-600 text-white px-6 py-3 rounded-lg transition flex items-center font-semibold"
          >
            <Plus className="w-5 h-5 mr-2" />
            Add Device
          </button>
        </div>

        {/* Stats */}
        <div className="grid md:grid-cols-3 gap-6 mb-8">
          <div className="bg-slate-800/50 backdrop-blur-sm border border-slate-700 rounded-xl p-6">
            <div className="flex items-center space-x-4">
              <div className="bg-blue-500/20 p-3 rounded-lg">
                <Monitor className="w-8 h-8 text-blue-400" />
              </div>
              <div>
                <div className="text-3xl font-bold text-white">{devices.length}</div>
                <div className="text-slate-400">Total Devices</div>
              </div>
            </div>
          </div>
          <div className="bg-slate-800/50 backdrop-blur-sm border border-slate-700 rounded-xl p-6">
            <div className="flex items-center space-x-4">
              <div className="bg-green-500/20 p-3 rounded-lg">
                <Check className="w-8 h-8 text-green-400" />
              </div>
              <div>
                <div className="text-3xl font-bold text-white">
                  {devices.filter(d => d.activated).length}
                </div>
                <div className="text-slate-400">Activated</div>
              </div>
            </div>
          </div>
          <div className="bg-slate-800/50 backdrop-blur-sm border border-slate-700 rounded-xl p-6">
            <div className="flex items-center space-x-4">
              <div className="bg-red-500/20 p-3 rounded-lg">
                <X className="w-8 h-8 text-red-400" />
              </div>
              <div>
                <div className="text-3xl font-bold text-white">
                  {devices.filter(d => !d.activated).length}
                </div>
                <div className="text-slate-400">Pending Activation</div>
              </div>
            </div>
          </div>
        </div>

        {/* Devices Table */}
        <div className="bg-slate-800/50 backdrop-blur-sm border border-slate-700 rounded-xl overflow-hidden">
          <div className="p-6 border-b border-slate-700">
            <h2 className="text-xl font-bold text-white flex items-center">
              <Cpu className="w-5 h-5 mr-2" />
              Registered Devices
            </h2>
          </div>
          
          {devices.length === 0 ? (
            <div className="p-12 text-center">
              <Cpu className="w-16 h-16 text-slate-600 mx-auto mb-4" />
              <h3 className="text-xl font-semibold text-slate-400 mb-2">No devices registered</h3>
              <p className="text-slate-500 mb-6">Add your first device to get started</p>
              <button
                onClick={() => setShowAddModal(true)}
                className="bg-orange-500 hover:bg-orange-600 text-white px-6 py-2 rounded-lg transition inline-flex items-center"
              >
                <Plus className="w-4 h-4 mr-2" />
                Add Device
              </button>
            </div>
          ) : (
            <div className="overflow-x-auto">
              <table className="w-full">
                <thead className="bg-slate-900/50">
                  <tr>
                    <th className="px-6 py-4 text-left text-sm font-semibold text-slate-300">#</th>
                    <th className="px-6 py-4 text-left text-sm font-semibold text-slate-300">Device Name</th>
                    <th className="px-6 py-4 text-left text-sm font-semibold text-slate-300">MAC Address</th>
                    <th className="px-6 py-4 text-left text-sm font-semibold text-slate-300">Status</th>
                    <th className="px-6 py-4 text-left text-sm font-semibold text-slate-300">Registered</th>
                    <th className="px-6 py-4 text-left text-sm font-semibold text-slate-300">Last Seen</th>
                    <th className="px-6 py-4 text-left text-sm font-semibold text-slate-300">Actions</th>
                  </tr>
                </thead>
                <tbody className="divide-y divide-slate-700">
                  {devices.map((device, index) => (
                    <tr key={device.id} className="hover:bg-slate-700/30 transition">
                      <td className="px-6 py-4 text-slate-400">{index + 1}</td>
                      <td className="px-6 py-4">
                        {editingId === device.id ? (
                          <div className="flex items-center space-x-2">
                            <input
                              type="text"
                              value={editName}
                              onChange={(e) => setEditName(e.target.value)}
                              className="bg-slate-900 border border-slate-600 rounded px-3 py-1 text-white focus:outline-none focus:border-orange-500"
                              autoFocus
                            />
                            <button
                              onClick={() => saveEdit(device.id)}
                              className="text-green-400 hover:text-green-300"
                            >
                              <Save className="w-4 h-4" />
                            </button>
                          </div>
                        ) : (
                          <div className="flex items-center space-x-2">
                            <span className="text-white font-medium">{device.name}</span>
                            <button
                              onClick={() => startEditing(device)}
                              className="text-slate-500 hover:text-slate-300"
                            >
                              <Edit2 className="w-4 h-4" />
                            </button>
                          </div>
                        )}
                      </td>
                      <td className="px-6 py-4">
                        <code className="text-orange-400 bg-slate-900/50 px-2 py-1 rounded text-sm">
                          {device.macAddress}
                        </code>
                      </td>
                      <td className="px-6 py-4">
                        <button
                          onClick={() => toggleActivation(device.id)}
                          className={`px-3 py-1 rounded-full text-sm font-medium flex items-center space-x-1 transition ${
                            device.activated
                              ? 'bg-green-500/20 text-green-400 hover:bg-green-500/30'
                              : 'bg-red-500/20 text-red-400 hover:bg-red-500/30'
                          }`}
                        >
                          {device.activated ? (
                            <>
                              <Check className="w-3 h-3" />
                              <span>Activated</span>
                            </>
                          ) : (
                            <>
                              <X className="w-3 h-3" />
                              <span>Inactive</span>
                            </>
                          )}
                        </button>
                      </td>
                      <td className="px-6 py-4 text-slate-400 text-sm">
                        {formatDate(device.createdAt)}
                      </td>
                      <td className="px-6 py-4 text-slate-400 text-sm">
                        {formatDate(device.lastSeen)}
                      </td>
                      <td className="px-6 py-4">
                        <button
                          onClick={() => deleteDevice(device.id)}
                          className="text-red-400 hover:text-red-300 transition"
                          title="Delete device"
                        >
                          <Trash2 className="w-5 h-5" />
                        </button>
                      </td>
                    </tr>
                  ))}
                </tbody>
              </table>
            </div>
          )}
        </div>

        {/* Info Box */}
        <div className="mt-8 bg-blue-500/10 border border-blue-500/30 rounded-xl p-6">
          <div className="flex items-start space-x-3">
            <Wifi className="w-6 h-6 text-blue-400 mt-1" />
            <div>
              <h3 className="text-blue-400 font-semibold mb-2">How Device Activation Works</h3>
              <ul className="text-slate-400 text-sm space-y-1">
                <li>• Add your device by entering its MAC address (found in device settings)</li>
                <li>• Click the status badge to toggle activation</li>
                <li>• Only activated devices can sync data with the cloud</li>
                <li>• Devices will show "Last Seen" when they connect to the server</li>
              </ul>
            </div>
          </div>
        </div>
      </div>

      {/* Add Device Modal */}
      {showAddModal && (
        <div className="fixed inset-0 bg-black/70 backdrop-blur-sm flex items-center justify-center z-50">
          <div className="bg-slate-800 border border-slate-700 rounded-xl p-8 w-full max-w-md mx-4">
            <h2 className="text-2xl font-bold text-white mb-6 flex items-center">
              <Plus className="w-6 h-6 mr-2" />
              Add New Device
            </h2>

            {error && (
              <div className="bg-red-500/20 border border-red-500/50 text-red-400 px-4 py-3 rounded-lg mb-4">
                {error}
              </div>
            )}

            <div className="space-y-4">
              <div>
                <label className="text-slate-400 text-sm mb-2 block">MAC Address *</label>
                <input
                  type="text"
                  value={newMac}
                  onChange={(e) => setNewMac(formatMac(e.target.value))}
                  placeholder="XX:XX:XX:XX:XX:XX"
                  className="w-full bg-slate-900 border border-slate-700 rounded-lg px-4 py-3 text-white font-mono focus:outline-none focus:border-orange-500"
                  maxLength={17}
                />
                <p className="text-slate-500 text-xs mt-1">
                  Find this in your device's Settings → WiFi Setup
                </p>
              </div>

              <div>
                <label className="text-slate-400 text-sm mb-2 block">Device Name (optional)</label>
                <input
                  type="text"
                  value={newName}
                  onChange={(e) => setNewName(e.target.value)}
                  placeholder="My Racing Device"
                  className="w-full bg-slate-900 border border-slate-700 rounded-lg px-4 py-3 text-white focus:outline-none focus:border-orange-500"
                />
              </div>
            </div>

            <div className="flex space-x-3 mt-6">
              <button
                onClick={() => {
                  setShowAddModal(false);
                  setNewMac('');
                  setNewName('');
                  setError('');
                }}
                className="flex-1 bg-slate-700 hover:bg-slate-600 text-white px-6 py-3 rounded-lg transition"
              >
                Cancel
              </button>
              <button
                onClick={handleAddDevice}
                className="flex-1 bg-orange-500 hover:bg-orange-600 text-white px-6 py-3 rounded-lg transition flex items-center justify-center"
              >
                <Plus className="w-4 h-4 mr-2" />
                Add Device
              </button>
            </div>
          </div>
        </div>
      )}
    </div>
  );
}
