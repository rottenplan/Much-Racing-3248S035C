'use client';

import { useState } from 'react';

import { Mail, MessageSquare, Phone, Send, MapPin, Zap } from 'lucide-react';

export default function ContactPage() {
    const [formData, setFormData] = useState({
        name: '',
        email: '',
        subject: '',
        message: ''
    });
    const [sending, setSending] = useState(false);
    const [success, setSuccess] = useState(false);

    const handleSubmit = (e: React.FormEvent) => {
        e.preventDefault();
        setSending(true);
        // Simulate API call
        setTimeout(() => {
            setSending(false);
            setSuccess(true);
            // Reset form after delay
            setTimeout(() => setSuccess(false), 3000);
            setFormData({ name: '', email: '', subject: '', message: '' });
        }, 1500);
    };

    return (
        <div className="min-h-screen bg-background text-foreground pb-24">


            <div className="container mx-auto px-4 py-6 space-y-6">

                {/* Contact Info Cards */}
                <div className="grid md:grid-cols-3 gap-4">
                    <div className="carbon-bg border border-border-color rounded-xl p-6 text-center hover:border-primary/50 transition">
                        <div className="w-12 h-12 bg-primary/20 rounded-full flex items-center justify-center mx-auto mb-4 text-primary">
                            <Mail className="w-6 h-6" />
                        </div>
                        <h3 className="font-racing text-lg mb-1">EMAIL SUPPORT</h3>
                        <p className="text-text-secondary text-sm">support@muchracing.com</p>
                    </div>
                    <div className="carbon-bg border border-border-color rounded-xl p-6 text-center hover:border-highlight/50 transition">
                        <div className="w-12 h-12 bg-highlight/20 rounded-full flex items-center justify-center mx-auto mb-4 text-highlight">
                            <Phone className="w-6 h-6" />
                        </div>
                        <h3 className="font-racing text-lg mb-1">PHONE</h3>
                        <p className="text-text-secondary text-sm">+62 812 3456 7890</p>
                    </div>
                    <div className="carbon-bg border border-border-color rounded-xl p-6 text-center hover:border-warning/50 transition">
                        <div className="w-12 h-12 bg-warning/20 rounded-full flex items-center justify-center mx-auto mb-4 text-warning">
                            <MapPin className="w-6 h-6" />
                        </div>
                        <h3 className="font-racing text-lg mb-1">OFFICE</h3>
                        <p className="text-text-secondary text-sm">Sentul International Circuit<br />Bogor, Indonesia</p>
                    </div>
                </div>

                {/* Contact Form */}
                <div className="carbon-bg border border-border-color rounded-xl p-6">
                    <h2 className="text-xl font-racing mb-6 flex items-center gap-2">
                        <MessageSquare className="w-5 h-5 text-primary" />
                        SEND MESSAGE
                    </h2>

                    <form onSubmit={handleSubmit} className="space-y-4">
                        <div className="grid md:grid-cols-2 gap-4">
                            <div>
                                <label className="block text-text-secondary text-xs font-medium mb-2">YOUR NAME</label>
                                <input
                                    type="text"
                                    required
                                    value={formData.name}
                                    onChange={e => setFormData({ ...formData, name: e.target.value })}
                                    className="w-full bg-background-secondary border border-border-color rounded-lg px-4 py-3 text-foreground focus:outline-none focus:border-primary transition"
                                    placeholder="John Doe"
                                />
                            </div>
                            <div>
                                <label className="block text-text-secondary text-xs font-medium mb-2">EMAIL ADDRESS</label>
                                <input
                                    type="email"
                                    required
                                    value={formData.email}
                                    onChange={e => setFormData({ ...formData, email: e.target.value })}
                                    className="w-full bg-background-secondary border border-border-color rounded-lg px-4 py-3 text-foreground focus:outline-none focus:border-primary transition"
                                    placeholder="john@example.com"
                                />
                            </div>
                        </div>

                        <div>
                            <label className="block text-text-secondary text-xs font-medium mb-2">SUBJECT</label>
                            <select
                                className="w-full bg-background-secondary border border-border-color rounded-lg px-4 py-3 text-foreground focus:outline-none focus:border-primary transition"
                                value={formData.subject}
                                onChange={e => setFormData({ ...formData, subject: e.target.value })}
                            >
                                <option value="">Select a topic</option>
                                <option value="support">Technical Support</option>
                                <option value="sales">Sales & Billing</option>
                                <option value="partnership">Partnership</option>
                                <option value="other">Other</option>
                            </select>
                        </div>

                        <div>
                            <label className="block text-text-secondary text-xs font-medium mb-2">MESSAGE</label>
                            <textarea
                                required
                                rows={5}
                                value={formData.message}
                                onChange={e => setFormData({ ...formData, message: e.target.value })}
                                className="w-full bg-background-secondary border border-border-color rounded-lg px-4 py-3 text-foreground focus:outline-none focus:border-primary transition"
                                placeholder="How can we help you?"
                            ></textarea>
                        </div>

                        <div className="flex justify-end">
                            <button
                                type="submit"
                                disabled={sending || success}
                                className={`
                            px-8 py-3 rounded-lg font-racing flex items-center gap-2 transition shadow-lg
                            ${success ? 'bg-highlight hover:bg-highlight text-white' : 'bg-primary hover:bg-primary-hover text-white'}
                            ${sending ? 'opacity-70 cursor-wait' : ''}
                        `}
                            >
                                {success ? (
                                    <>SENT SUCCESSFULLY!</>
                                ) : (
                                    <>
                                        {sending ? 'SENDING...' : 'SEND MESSAGE'}
                                        {!sending && <Send className="w-4 h-4" />}
                                    </>
                                )}
                            </button>
                        </div>
                    </form>
                </div>

            </div>
        </div>
    );
}
