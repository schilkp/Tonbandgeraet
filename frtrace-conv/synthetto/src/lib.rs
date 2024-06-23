use prost::Message;

mod protos {
    #![allow(clippy::large_enum_variant)]
    #![allow(clippy::enum_variant_names)]
    include!(concat!(env!("OUT_DIR"), "/perfetto.protos.rs"));
}

const TRUSTED_PACKET_SEQUENCE_ID: Option<protos::trace_packet::OptionalTrustedPacketSequenceId> =
    Some(protos::trace_packet::OptionalTrustedPacketSequenceId::TrustedPacketSequenceId(0xDEADBEEF));

pub trait Uuid {
    fn uuid(&self) -> u64;
}

pub use protos::TracePacket;

pub struct Synthetto {
    uuid_cnt: u64,
    track_descriptors: Vec<protos::TrackDescriptor>,
    last_emited_descriptor: Option<usize>,
}

impl Default for Synthetto {
    fn default() -> Self {
        Self::new()
    }
}

impl Synthetto {
    pub fn new() -> Self {
        Synthetto {
            uuid_cnt: 1,
            track_descriptors: vec![],
            last_emited_descriptor: None,
        }
    }

    pub fn next_uuid(&mut self) -> u64 {
        let uuid = self.uuid_cnt;
        self.uuid_cnt += 1;
        uuid
    }

    pub fn new_process(
        &mut self,
        pid: i32,
        process_name: String,
        cmdline: Vec<String>,
        priority: Option<i32>,
    ) -> Process {
        let uuid = self.next_uuid();

        let evt = protos::TrackDescriptor {
            uuid: Some(uuid),
            parent_uuid: None,
            process: Some(protos::ProcessDescriptor {
                pid: Some(pid),
                cmdline,
                process_name: Some(process_name),
                process_priority: priority,
                start_timestamp_ns: None,
            }),
            thread: None,
            counter: None,
            static_or_dynamic_name: None,
        };

        self.track_descriptors.push(evt);

        Process { uuid, pid }
    }

    pub fn new_thread(&mut self, process: &Process, tid: i32, thread_name: String) -> Thread {
        let uuid = self.next_uuid();

        let evt = protos::TrackDescriptor {
            uuid: Some(uuid),
            parent_uuid: Some(process.uuid),
            process: None,
            thread: Some(protos::ThreadDescriptor {
                pid: Some(process.pid),
                tid: Some(tid),
                thread_name: Some(thread_name),
            }),
            counter: None,
            static_or_dynamic_name: None,
        };

        self.track_descriptors.push(evt);

        Thread { uuid }
    }

    fn generate_track_descriptor(&mut self, name: String, parent_uuid: Option<u64>) -> u64 {
        let uuid = self.next_uuid();
        let evt = protos::TrackDescriptor {
            uuid: Some(uuid),
            parent_uuid,
            process: None,
            thread: None,
            counter: None,
            static_or_dynamic_name: Some(protos::track_descriptor::StaticOrDynamicName::Name(name)),
        };
        self.track_descriptors.push(evt);
        uuid
    }

    fn generate_counter_track_descriptor(
        &mut self,
        name: String,
        unit: CounterTrackUnit,
        unit_mult: i64,
        is_incremental: bool,
        parent_uuid: Option<u64>,
    ) -> u64 {
        let uuid = self.next_uuid();
        let evt = protos::TrackDescriptor {
            uuid: Some(uuid),
            parent_uuid,
            process: None,
            thread: None,
            counter: Some(protos::CounterDescriptor {
                categories: vec![],
                unit: unit.to_proto_unit(),
                unit_name: unit.to_proto_unit_name(),
                unit_multiplier: Some(unit_mult),
                is_incremental: Some(is_incremental),
            }),
            static_or_dynamic_name: Some(protos::track_descriptor::StaticOrDynamicName::Name(name)),
        };
        self.track_descriptors.push(evt);
        uuid
    }

    pub fn new_global_track(&mut self, name: String) -> Track<Global, EventTrack> {
        let uuid = self.generate_track_descriptor(name.clone(), None);
        Track {
            scope: std::marker::PhantomData,
            kind: std::marker::PhantomData,
            uuid,
            pid: 0, // Don't care
            name,
        }
    }

    pub fn new_global_counter_track(
        &mut self,
        name: String,
        unit: CounterTrackUnit,
        unit_mult: i64,
        is_incremental: bool,
    ) -> Track<Global, CounterTrack> {
        let uuid = self.generate_counter_track_descriptor(name.clone(), unit, unit_mult, is_incremental, None);
        Track {
            scope: std::marker::PhantomData,
            kind: std::marker::PhantomData,
            uuid,
            pid: 0, // Don't care
            name,
        }
    }

    pub fn new_process_track(&mut self, name: String, process: &Process) -> Track<Process, EventTrack> {
        let uuid = self.generate_track_descriptor(name.clone(), Some(process.uuid));
        Track {
            scope: std::marker::PhantomData,
            kind: std::marker::PhantomData,
            uuid,
            pid: process.pid,
            name,
        }
    }

    pub fn new_stacked_process_track(&mut self, t: &Track<Process, EventTrack>) -> Track<Process, EventTrack> {
        let uuid = self.generate_track_descriptor(t.name.clone(), Some(t.uuid));
        Track {
            scope: std::marker::PhantomData,
            kind: std::marker::PhantomData,
            uuid,
            pid: t.pid,
            name: t.name.clone(),
        }
    }

    pub fn new_process_counter_track(
        &mut self,
        name: String,
        unit: CounterTrackUnit,
        unit_mult: i64,
        is_incremental: bool,
        process: &Process,
    ) -> Track<Global, CounterTrack> {
        let uuid =
            self.generate_counter_track_descriptor(name.clone(), unit, unit_mult, is_incremental, Some(process.uuid));
        Track {
            scope: std::marker::PhantomData,
            kind: std::marker::PhantomData,
            uuid,
            pid: process.pid,
            name,
        }
    }

    pub fn new_thread_track(&mut self, name: String, thread: &Thread) -> Track<Thread, EventTrack> {
        let uuid = self.generate_track_descriptor(name.clone(), Some(thread.uuid));
        Track {
            scope: std::marker::PhantomData,
            kind: std::marker::PhantomData,
            uuid,
            pid: 0, // Don't care
            name,
        }
    }

    pub fn new_thread_counter_track(
        &mut self,
        name: String,
        unit: CounterTrackUnit,
        unit_mult: i64,
        is_incremental: bool,
        thread: &Thread,
    ) -> Track<Global, CounterTrack> {
        let uuid =
            self.generate_counter_track_descriptor(name.clone(), unit, unit_mult, is_incremental, Some(thread.uuid));
        Track {
            scope: std::marker::PhantomData,
            kind: std::marker::PhantomData,
            uuid,
            pid: 0, // Don't care
            name,
        }
    }

    pub fn complete_descriptor_trace(&self) -> Vec<TracePacket> {
        let mut r = vec![];

        for descr in &self.track_descriptors {
            let evt = protos::TracePacket {
                data: Some(protos::trace_packet::Data::TrackDescriptor(descr.clone())),
                ..protos::TracePacket::default()
            };
            r.push(evt)
        }

        r
    }

    pub fn new_descriptor_trace_evts(&mut self) -> Vec<TracePacket> {
        let mut r = vec![];

        let first_idx = self.last_emited_descriptor.map(|x| x + 1).unwrap_or(0);

        if first_idx < self.track_descriptors.len() {
            for idx in first_idx..self.track_descriptors.len() {
                let evt = protos::TracePacket {
                    data: Some(protos::trace_packet::Data::TrackDescriptor(self.track_descriptors[idx].clone())),
                    ..protos::TracePacket::default()
                };
                r.push(evt)
            }
        }

        if !self.track_descriptors.is_empty() {
            self.last_emited_descriptor = Some(self.track_descriptors.len() - 1);
        }

        r
    }
}

pub struct Track<S, K>
where
    S: TrackScope,
    K: TrackType,
{
    name: String,
    pid: i32, // Must be set if TrackScope == Process
    scope: std::marker::PhantomData<S>,
    kind: std::marker::PhantomData<K>,
    uuid: u64,
}

// == Scopes ==

pub trait TrackScope {}

pub struct Global {}

impl TrackScope for Global {}

pub struct Process {
    uuid: u64,
    pid: i32,
}

impl TrackScope for Process {}

pub struct Thread {
    uuid: u64,
}

impl TrackScope for Thread {}

// == Track Types ==

pub trait TrackType {}

pub struct EventTrack {}

impl TrackType for EventTrack {}

pub struct CounterTrack {}

impl TrackType for CounterTrack {}

// == Event Tracks ==

impl<S: TrackScope> Track<S, EventTrack> {
    pub fn slice_begin_evt(&self, ts: u64, name: Option<String>) -> TracePacket {
        TracePacket {
            timestamp: Some(ts),
            data: Some(protos::trace_packet::Data::TrackEvent(protos::TrackEvent {
                name_field: name.map(protos::track_event::NameField::Name),
                track_uuid: Some(self.uuid),
                r#type: Some(protos::track_event::Type::SliceBegin as i32),
                ..protos::TrackEvent::default()
            })),
            optional_trusted_packet_sequence_id: TRUSTED_PACKET_SEQUENCE_ID.clone(),
            ..protos::TracePacket::default()
        }
    }

    pub fn slice_end_evt(&self, ts: u64) -> TracePacket {
        protos::TracePacket {
            timestamp: Some(ts),
            data: Some(protos::trace_packet::Data::TrackEvent(protos::TrackEvent {
                name_field: None,
                track_uuid: Some(self.uuid),
                r#type: Some(protos::track_event::Type::SliceEnd as i32),
                ..protos::TrackEvent::default()
            })),
            optional_trusted_packet_sequence_id: TRUSTED_PACKET_SEQUENCE_ID.clone(),
            ..protos::TracePacket::default()
        }
    }

    pub fn instant_evt(&self, ts: u64, name: String) -> TracePacket {
        protos::TracePacket {
            timestamp: Some(ts),
            data: Some(protos::trace_packet::Data::TrackEvent(protos::TrackEvent {
                name_field: Some(protos::track_event::NameField::Name(name)),
                track_uuid: Some(self.uuid),
                r#type: Some(protos::track_event::Type::Instant as i32),
                ..protos::TrackEvent::default()
            })),
            optional_trusted_packet_sequence_id: TRUSTED_PACKET_SEQUENCE_ID.clone(),
            ..protos::TracePacket::default()
        }
    }
}

// == Counter Tracks ==

impl<S: TrackScope> Track<S, CounterTrack> {
    pub fn int_counter_evt<V>(&self, ts: u64, val: V) -> TracePacket
    where
        V: Into<i64>,
    {
        protos::TracePacket {
            timestamp: Some(ts),
            data: Some(protos::trace_packet::Data::TrackEvent(protos::TrackEvent {
                track_uuid: Some(self.uuid),
                r#type: Some(protos::track_event::Type::Counter as i32),
                counter_value_field: Some(protos::track_event::CounterValueField::CounterValue(val.into())),
                ..protos::TrackEvent::default()
            })),
            optional_trusted_packet_sequence_id: TRUSTED_PACKET_SEQUENCE_ID.clone(),
            ..protos::TracePacket::default()
        }
    }

    pub fn float_counter_evt<V>(&self, ts: u64, val: V) -> TracePacket
    where
        V: Into<f64>,
    {
        protos::TracePacket {
            timestamp: Some(ts),
            data: Some(protos::trace_packet::Data::TrackEvent(protos::TrackEvent {
                track_uuid: Some(self.uuid),
                r#type: Some(protos::track_event::Type::Counter as i32),
                counter_value_field: Some(protos::track_event::CounterValueField::DoubleCounterValue(val.into())),
                ..protos::TrackEvent::default()
            })),
            optional_trusted_packet_sequence_id: TRUSTED_PACKET_SEQUENCE_ID.clone(),
            ..protos::TracePacket::default()
        }
    }
}

pub enum CounterTrackUnit {
    Unspecified,
    TimeNs,
    Count,
    SizeBytes,
    Custom(String),
}

impl CounterTrackUnit {
    fn to_proto_unit(&self) -> Option<i32> {
        Some(match self {
            CounterTrackUnit::Unspecified => protos::counter_descriptor::Unit::Unspecified,
            CounterTrackUnit::TimeNs => protos::counter_descriptor::Unit::TimeNs,
            CounterTrackUnit::Count => protos::counter_descriptor::Unit::Count,
            CounterTrackUnit::SizeBytes => protos::counter_descriptor::Unit::SizeBytes,
            CounterTrackUnit::Custom(_) => protos::counter_descriptor::Unit::Unspecified,
        } as i32)
    }

    fn to_proto_unit_name(&self) -> Option<String> {
        if let Self::Custom(name) = self {
            Some(name.clone())
        } else {
            None
        }
    }
}

pub fn encode_trace(i: Vec<TracePacket>) -> Vec<u8> {
    protos::Trace { packet: i }.encode_to_vec()
}
