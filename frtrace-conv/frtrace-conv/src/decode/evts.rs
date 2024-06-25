//! # Important:
//!
//! ```text
//!         ____   ___    _   _  ___ _____   _____ ____ ___ _____
//!        |  _ \ / _ \  | \ | |/ _ \_   _| | ____|  _ \_ _|_   _|
//!        | | | | | | | |  \| | | | || |   |  _| | | | | |  | |
//!        | |_| | |_| | | |\  | |_| || |   | |___| |_| | |  | |
//!        |____/ \___/  |_| \_|\___/ |_|   |_____|____/___| |_|
//!
//! ```
//!
//! This file is generated automatically. See `code_gen` folder in repo.
use std::rc::Rc;

use anyhow::{anyhow, Context};

use crate::decode::{bytes_left, decode_s64, decode_string, decode_u32, decode_u64, decode_u8};

#[derive(Debug, Clone)]
pub enum RawEvt {
    Metadata(RawMetadataEvt),
    Trace(RawTraceEvt),
    Invalid(RawInvalidEvt),
}

impl RawEvt {
    pub fn ts(&self) -> Option<u64> {
        match self {
            RawEvt::Metadata(_) => None,
            RawEvt::Trace(e) => Some(e.ts),
            RawEvt::Invalid(e) => e.ts,
        }
    }
}

#[derive(Debug, Clone)]
pub struct RawInvalidEvt {
    pub ts: Option<u64>,
    pub err: Option<Rc<anyhow::Error>>,
}

#[derive(Debug, Clone)]
pub struct RawTraceEvt {
    pub ts: u64,
    pub kind: RawTraceEvtKind,
}

// ==== Enums ==================================================================

#[derive(Debug, Clone)]
pub enum QueueKind {
    QkQueue,
    QkCountingSemphr,
    QkBinarySemphr,
    QkMutex,
    QkRecursiveMutex,
    QkQueueSet,
}

impl TryFrom<u8> for QueueKind {
    type Error = anyhow::Error;

    fn try_from(value: u8) -> Result<Self, Self::Error> {
        match value {
            0 => Ok(Self::QkQueue),
            1 => Ok(Self::QkCountingSemphr),
            2 => Ok(Self::QkBinarySemphr),
            3 => Ok(Self::QkMutex),
            4 => Ok(Self::QkRecursiveMutex),
            5 => Ok(Self::QkQueueSet),
            _ => Err(anyhow!("Invalid QueueKind")),
        }
    }
}

#[derive(Debug, Clone)]
pub enum StreamBufferKind {
    SbkStreamBuffer,
    SbkMessageBuffer,
}

impl TryFrom<u8> for StreamBufferKind {
    type Error = anyhow::Error;

    fn try_from(value: u8) -> Result<Self, Self::Error> {
        match value {
            0 => Ok(Self::SbkStreamBuffer),
            1 => Ok(Self::SbkMessageBuffer),
            _ => Err(anyhow!("Invalid StreamBufferKind")),
        }
    }
}

// ==== Event Types ============================================================

#[derive(Debug, Clone)]
pub enum RawMetadataEvt {
    TsResolutionNs(TsResolutionNsEvt),
    TaskName(TaskNameEvt),
    TaskIsIdleTask(TaskIsIdleTaskEvt),
    TaskIsTimerTask(TaskIsTimerTaskEvt),
    IsrName(IsrNameEvt),
    QueueName(QueueNameEvt),
    QueueKind(QueueKindEvt),
    EvtmarkerName(EvtmarkerNameEvt),
    ValmarkerName(ValmarkerNameEvt),
    TaskEvtmarkerName(TaskEvtmarkerNameEvt),
    TaskValmarkerName(TaskValmarkerNameEvt),
}

#[derive(Debug, Clone)]
pub enum RawTraceEvtKind {
    CoreId(CoreIdEvt),
    DroppedEvtCnt(DroppedEvtCntEvt),
    TaskSwitchedIn(TaskSwitchedInEvt),
    TaskToRdyState(TaskToRdyStateEvt),
    TaskResumed(TaskResumedEvt),
    TaskResumedFromIsr(TaskResumedFromIsrEvt),
    TaskSuspended(TaskSuspendedEvt),
    CurtaskDelay(CurtaskDelayEvt),
    CurtaskDelayUntil(CurtaskDelayUntilEvt),
    TaskPrioritySet(TaskPrioritySetEvt),
    TaskPriorityInherit(TaskPriorityInheritEvt),
    TaskPriorityDisinherit(TaskPriorityDisinheritEvt),
    TaskCreated(TaskCreatedEvt),
    TaskDeleted(TaskDeletedEvt),
    IsrEnter(IsrEnterEvt),
    IsrExit(IsrExitEvt),
    QueueCreated(QueueCreatedEvt),
    QueueSend(QueueSendEvt),
    QueueSendFromIsr(QueueSendFromIsrEvt),
    QueueOverwrite(QueueOverwriteEvt),
    QueueOverwriteFromIsr(QueueOverwriteFromIsrEvt),
    QueueReceive(QueueReceiveEvt),
    QueueReceiveFromIsr(QueueReceiveFromIsrEvt),
    QueueReset(QueueResetEvt),
    CurtaskBlockOnQueuePeek(CurtaskBlockOnQueuePeekEvt),
    CurtaskBlockOnQueueSend(CurtaskBlockOnQueueSendEvt),
    CurtaskBlockOnQueueReceive(CurtaskBlockOnQueueReceiveEvt),
    Evtmarker(EvtmarkerEvt),
    EvtmarkerBegin(EvtmarkerBeginEvt),
    EvtmarkerEnd(EvtmarkerEndEvt),
    Valmarker(ValmarkerEvt),
    TaskEvtmarker(TaskEvtmarkerEvt),
    TaskEvtmarkerBegin(TaskEvtmarkerBeginEvt),
    TaskEvtmarkerEnd(TaskEvtmarkerEndEvt),
    TaskValmarker(TaskValmarkerEvt),
}

impl RawEvt {
    pub fn decode(buf: &[u8]) -> anyhow::Result<Self> {
        let mut idx = 0;
        let id = decode_u8(buf, &mut idx)?;
        match id {
            0 => CoreIdEvt::decode(buf, &mut idx),
            1 => DroppedEvtCntEvt::decode(buf, &mut idx),
            2 => TsResolutionNsEvt::decode(buf, &mut idx),
            3 => TaskSwitchedInEvt::decode(buf, &mut idx),
            4 => TaskToRdyStateEvt::decode(buf, &mut idx),
            5 => TaskResumedEvt::decode(buf, &mut idx),
            6 => TaskResumedFromIsrEvt::decode(buf, &mut idx),
            7 => TaskSuspendedEvt::decode(buf, &mut idx),
            8 => CurtaskDelayEvt::decode(buf, &mut idx),
            9 => CurtaskDelayUntilEvt::decode(buf, &mut idx),
            10 => TaskPrioritySetEvt::decode(buf, &mut idx),
            11 => TaskPriorityInheritEvt::decode(buf, &mut idx),
            12 => TaskPriorityDisinheritEvt::decode(buf, &mut idx),
            13 => TaskCreatedEvt::decode(buf, &mut idx),
            14 => TaskNameEvt::decode(buf, &mut idx),
            15 => TaskIsIdleTaskEvt::decode(buf, &mut idx),
            16 => TaskIsTimerTaskEvt::decode(buf, &mut idx),
            17 => TaskDeletedEvt::decode(buf, &mut idx),
            18 => IsrNameEvt::decode(buf, &mut idx),
            19 => IsrEnterEvt::decode(buf, &mut idx),
            20 => IsrExitEvt::decode(buf, &mut idx),
            21 => QueueCreatedEvt::decode(buf, &mut idx),
            22 => QueueNameEvt::decode(buf, &mut idx),
            23 => QueueKindEvt::decode(buf, &mut idx),
            24 => QueueSendEvt::decode(buf, &mut idx),
            25 => QueueSendFromIsrEvt::decode(buf, &mut idx),
            26 => QueueOverwriteEvt::decode(buf, &mut idx),
            27 => QueueOverwriteFromIsrEvt::decode(buf, &mut idx),
            28 => QueueReceiveEvt::decode(buf, &mut idx),
            29 => QueueReceiveFromIsrEvt::decode(buf, &mut idx),
            30 => QueueResetEvt::decode(buf, &mut idx),
            31 => CurtaskBlockOnQueuePeekEvt::decode(buf, &mut idx),
            32 => CurtaskBlockOnQueueSendEvt::decode(buf, &mut idx),
            33 => CurtaskBlockOnQueueReceiveEvt::decode(buf, &mut idx),
            34 => EvtmarkerNameEvt::decode(buf, &mut idx),
            35 => EvtmarkerEvt::decode(buf, &mut idx),
            36 => EvtmarkerBeginEvt::decode(buf, &mut idx),
            37 => EvtmarkerEndEvt::decode(buf, &mut idx),
            38 => ValmarkerNameEvt::decode(buf, &mut idx),
            39 => ValmarkerEvt::decode(buf, &mut idx),
            40 => TaskEvtmarkerNameEvt::decode(buf, &mut idx),
            41 => TaskEvtmarkerEvt::decode(buf, &mut idx),
            42 => TaskEvtmarkerBeginEvt::decode(buf, &mut idx),
            43 => TaskEvtmarkerEndEvt::decode(buf, &mut idx),
            44 => TaskValmarkerNameEvt::decode(buf, &mut idx),
            45 => TaskValmarkerEvt::decode(buf, &mut idx),
            _ => Err(anyhow!("Invalid event id {id}"))?,
        }
    }
}

// ==== Individual Events ======================================================

#[derive(Debug, Clone)]
pub struct CoreIdEvt {
    pub core_id: u32,
}

impl CoreIdEvt {
    fn decode(buf: &[u8], current_idx: &mut usize) -> anyhow::Result<RawEvt> {
        let ts = decode_u64(buf, current_idx)?;
        let core_id = decode_u32(buf, current_idx).context("Failed to decode 'core_id' u32 field.")?;
        if bytes_left(buf, *current_idx) {
            return Err(anyhow!("Loose bytes at end of 'CoreId' event."));
        }
        Ok(RawEvt::Trace(RawTraceEvt {
            ts,
            kind: RawTraceEvtKind::CoreId(Self { core_id }),
        }))
    }
}

#[derive(Debug, Clone)]
pub struct DroppedEvtCntEvt {
    pub cnt: u32,
}

impl DroppedEvtCntEvt {
    fn decode(buf: &[u8], current_idx: &mut usize) -> anyhow::Result<RawEvt> {
        let ts = decode_u64(buf, current_idx)?;
        let cnt = decode_u32(buf, current_idx).context("Failed to decode 'cnt' u32 field.")?;
        if bytes_left(buf, *current_idx) {
            return Err(anyhow!("Loose bytes at end of 'DroppedEvtCnt' event."));
        }
        Ok(RawEvt::Trace(RawTraceEvt {
            ts,
            kind: RawTraceEvtKind::DroppedEvtCnt(Self { cnt }),
        }))
    }
}

#[derive(Debug, Clone)]
pub struct TsResolutionNsEvt {
    pub ns_per_ts: u64,
}

impl TsResolutionNsEvt {
    fn decode(buf: &[u8], current_idx: &mut usize) -> anyhow::Result<RawEvt> {
        let ns_per_ts = decode_u64(buf, current_idx).context("Failed to decode 'ns_per_ts' u64 field.")?;
        if bytes_left(buf, *current_idx) {
            return Err(anyhow!("Loose bytes at end of 'TsResolutionNs' event."));
        }
        Ok(RawEvt::Metadata(RawMetadataEvt::TsResolutionNs(Self { ns_per_ts })))
    }
}

#[derive(Debug, Clone)]
pub struct TaskSwitchedInEvt {
    pub task_id: u32,
}

impl TaskSwitchedInEvt {
    fn decode(buf: &[u8], current_idx: &mut usize) -> anyhow::Result<RawEvt> {
        let ts = decode_u64(buf, current_idx)?;
        let task_id = decode_u32(buf, current_idx).context("Failed to decode 'task_id' u32 field.")?;
        if bytes_left(buf, *current_idx) {
            return Err(anyhow!("Loose bytes at end of 'TaskSwitchedIn' event."));
        }
        Ok(RawEvt::Trace(RawTraceEvt {
            ts,
            kind: RawTraceEvtKind::TaskSwitchedIn(Self { task_id }),
        }))
    }
}

#[derive(Debug, Clone)]
pub struct TaskToRdyStateEvt {
    pub task_id: u32,
}

impl TaskToRdyStateEvt {
    fn decode(buf: &[u8], current_idx: &mut usize) -> anyhow::Result<RawEvt> {
        let ts = decode_u64(buf, current_idx)?;
        let task_id = decode_u32(buf, current_idx).context("Failed to decode 'task_id' u32 field.")?;
        if bytes_left(buf, *current_idx) {
            return Err(anyhow!("Loose bytes at end of 'TaskToRdyState' event."));
        }
        Ok(RawEvt::Trace(RawTraceEvt {
            ts,
            kind: RawTraceEvtKind::TaskToRdyState(Self { task_id }),
        }))
    }
}

#[derive(Debug, Clone)]
pub struct TaskResumedEvt {
    pub task_id: u32,
}

impl TaskResumedEvt {
    fn decode(buf: &[u8], current_idx: &mut usize) -> anyhow::Result<RawEvt> {
        let ts = decode_u64(buf, current_idx)?;
        let task_id = decode_u32(buf, current_idx).context("Failed to decode 'task_id' u32 field.")?;
        if bytes_left(buf, *current_idx) {
            return Err(anyhow!("Loose bytes at end of 'TaskResumed' event."));
        }
        Ok(RawEvt::Trace(RawTraceEvt {
            ts,
            kind: RawTraceEvtKind::TaskResumed(Self { task_id }),
        }))
    }
}

#[derive(Debug, Clone)]
pub struct TaskResumedFromIsrEvt {
    pub task_id: u32,
}

impl TaskResumedFromIsrEvt {
    fn decode(buf: &[u8], current_idx: &mut usize) -> anyhow::Result<RawEvt> {
        let ts = decode_u64(buf, current_idx)?;
        let task_id = decode_u32(buf, current_idx).context("Failed to decode 'task_id' u32 field.")?;
        if bytes_left(buf, *current_idx) {
            return Err(anyhow!("Loose bytes at end of 'TaskResumedFromIsr' event."));
        }
        Ok(RawEvt::Trace(RawTraceEvt {
            ts,
            kind: RawTraceEvtKind::TaskResumedFromIsr(Self { task_id }),
        }))
    }
}

#[derive(Debug, Clone)]
pub struct TaskSuspendedEvt {
    pub task_id: u32,
}

impl TaskSuspendedEvt {
    fn decode(buf: &[u8], current_idx: &mut usize) -> anyhow::Result<RawEvt> {
        let ts = decode_u64(buf, current_idx)?;
        let task_id = decode_u32(buf, current_idx).context("Failed to decode 'task_id' u32 field.")?;
        if bytes_left(buf, *current_idx) {
            return Err(anyhow!("Loose bytes at end of 'TaskSuspended' event."));
        }
        Ok(RawEvt::Trace(RawTraceEvt {
            ts,
            kind: RawTraceEvtKind::TaskSuspended(Self { task_id }),
        }))
    }
}

#[derive(Debug, Clone)]
pub struct CurtaskDelayEvt {
    pub ticks: u32,
}

impl CurtaskDelayEvt {
    fn decode(buf: &[u8], current_idx: &mut usize) -> anyhow::Result<RawEvt> {
        let ts = decode_u64(buf, current_idx)?;
        let ticks = decode_u32(buf, current_idx).context("Failed to decode 'ticks' u32 field.")?;
        if bytes_left(buf, *current_idx) {
            return Err(anyhow!("Loose bytes at end of 'CurtaskDelay' event."));
        }
        Ok(RawEvt::Trace(RawTraceEvt {
            ts,
            kind: RawTraceEvtKind::CurtaskDelay(Self { ticks }),
        }))
    }
}

#[derive(Debug, Clone)]
pub struct CurtaskDelayUntilEvt {
    pub time_to_wake: u32,
}

impl CurtaskDelayUntilEvt {
    fn decode(buf: &[u8], current_idx: &mut usize) -> anyhow::Result<RawEvt> {
        let ts = decode_u64(buf, current_idx)?;
        let time_to_wake = decode_u32(buf, current_idx).context("Failed to decode 'time_to_wake' u32 field.")?;
        if bytes_left(buf, *current_idx) {
            return Err(anyhow!("Loose bytes at end of 'CurtaskDelayUntil' event."));
        }
        Ok(RawEvt::Trace(RawTraceEvt {
            ts,
            kind: RawTraceEvtKind::CurtaskDelayUntil(Self { time_to_wake }),
        }))
    }
}

#[derive(Debug, Clone)]
pub struct TaskPrioritySetEvt {
    pub task_id: u32,
    pub priority: u32,
}

impl TaskPrioritySetEvt {
    fn decode(buf: &[u8], current_idx: &mut usize) -> anyhow::Result<RawEvt> {
        let ts = decode_u64(buf, current_idx)?;
        let task_id = decode_u32(buf, current_idx).context("Failed to decode 'task_id' u32 field.")?;
        let priority = decode_u32(buf, current_idx).context("Failed to decode 'priority' u32 field.")?;
        if bytes_left(buf, *current_idx) {
            return Err(anyhow!("Loose bytes at end of 'TaskPrioritySet' event."));
        }
        Ok(RawEvt::Trace(RawTraceEvt {
            ts,
            kind: RawTraceEvtKind::TaskPrioritySet(Self { task_id, priority }),
        }))
    }
}

#[derive(Debug, Clone)]
pub struct TaskPriorityInheritEvt {
    pub task_id: u32,
    pub priority: u32,
}

impl TaskPriorityInheritEvt {
    fn decode(buf: &[u8], current_idx: &mut usize) -> anyhow::Result<RawEvt> {
        let ts = decode_u64(buf, current_idx)?;
        let task_id = decode_u32(buf, current_idx).context("Failed to decode 'task_id' u32 field.")?;
        let priority = decode_u32(buf, current_idx).context("Failed to decode 'priority' u32 field.")?;
        if bytes_left(buf, *current_idx) {
            return Err(anyhow!("Loose bytes at end of 'TaskPriorityInherit' event."));
        }
        Ok(RawEvt::Trace(RawTraceEvt {
            ts,
            kind: RawTraceEvtKind::TaskPriorityInherit(Self { task_id, priority }),
        }))
    }
}

#[derive(Debug, Clone)]
pub struct TaskPriorityDisinheritEvt {
    pub task_id: u32,
    pub priority: u32,
}

impl TaskPriorityDisinheritEvt {
    fn decode(buf: &[u8], current_idx: &mut usize) -> anyhow::Result<RawEvt> {
        let ts = decode_u64(buf, current_idx)?;
        let task_id = decode_u32(buf, current_idx).context("Failed to decode 'task_id' u32 field.")?;
        let priority = decode_u32(buf, current_idx).context("Failed to decode 'priority' u32 field.")?;
        if bytes_left(buf, *current_idx) {
            return Err(anyhow!("Loose bytes at end of 'TaskPriorityDisinherit' event."));
        }
        Ok(RawEvt::Trace(RawTraceEvt {
            ts,
            kind: RawTraceEvtKind::TaskPriorityDisinherit(Self { task_id, priority }),
        }))
    }
}

#[derive(Debug, Clone)]
pub struct TaskCreatedEvt {
    pub task_id: u32,
}

impl TaskCreatedEvt {
    fn decode(buf: &[u8], current_idx: &mut usize) -> anyhow::Result<RawEvt> {
        let ts = decode_u64(buf, current_idx)?;
        let task_id = decode_u32(buf, current_idx).context("Failed to decode 'task_id' u32 field.")?;
        if bytes_left(buf, *current_idx) {
            return Err(anyhow!("Loose bytes at end of 'TaskCreated' event."));
        }
        Ok(RawEvt::Trace(RawTraceEvt {
            ts,
            kind: RawTraceEvtKind::TaskCreated(Self { task_id }),
        }))
    }
}

#[derive(Debug, Clone)]
pub struct TaskNameEvt {
    pub task_id: u32,
    pub name: String,
}

impl TaskNameEvt {
    fn decode(buf: &[u8], current_idx: &mut usize) -> anyhow::Result<RawEvt> {
        let task_id = decode_u32(buf, current_idx).context("Failed to decode 'task_id' u32 field.")?;
        let name = decode_string(buf, current_idx)?;
        if bytes_left(buf, *current_idx) {
            return Err(anyhow!("Loose bytes at end of 'TaskName' event."));
        }
        Ok(RawEvt::Metadata(RawMetadataEvt::TaskName(Self { task_id, name })))
    }
}

#[derive(Debug, Clone)]
pub struct TaskIsIdleTaskEvt {
    pub task_id: u32,
    pub core_id: u32,
}

impl TaskIsIdleTaskEvt {
    fn decode(buf: &[u8], current_idx: &mut usize) -> anyhow::Result<RawEvt> {
        let task_id = decode_u32(buf, current_idx).context("Failed to decode 'task_id' u32 field.")?;
        let core_id = decode_u32(buf, current_idx).context("Failed to decode 'core_id' u32 field.")?;
        if bytes_left(buf, *current_idx) {
            return Err(anyhow!("Loose bytes at end of 'TaskIsIdleTask' event."));
        }
        Ok(RawEvt::Metadata(RawMetadataEvt::TaskIsIdleTask(Self { task_id, core_id })))
    }
}

#[derive(Debug, Clone)]
pub struct TaskIsTimerTaskEvt {
    pub task_id: u32,
}

impl TaskIsTimerTaskEvt {
    fn decode(buf: &[u8], current_idx: &mut usize) -> anyhow::Result<RawEvt> {
        let task_id = decode_u32(buf, current_idx).context("Failed to decode 'task_id' u32 field.")?;
        if bytes_left(buf, *current_idx) {
            return Err(anyhow!("Loose bytes at end of 'TaskIsTimerTask' event."));
        }
        Ok(RawEvt::Metadata(RawMetadataEvt::TaskIsTimerTask(Self { task_id })))
    }
}

#[derive(Debug, Clone)]
pub struct TaskDeletedEvt {
    pub task_id: u32,
}

impl TaskDeletedEvt {
    fn decode(buf: &[u8], current_idx: &mut usize) -> anyhow::Result<RawEvt> {
        let ts = decode_u64(buf, current_idx)?;
        let task_id = decode_u32(buf, current_idx).context("Failed to decode 'task_id' u32 field.")?;
        if bytes_left(buf, *current_idx) {
            return Err(anyhow!("Loose bytes at end of 'TaskDeleted' event."));
        }
        Ok(RawEvt::Trace(RawTraceEvt {
            ts,
            kind: RawTraceEvtKind::TaskDeleted(Self { task_id }),
        }))
    }
}

#[derive(Debug, Clone)]
pub struct IsrNameEvt {
    pub isr_id: u32,
    pub name: String,
}

impl IsrNameEvt {
    fn decode(buf: &[u8], current_idx: &mut usize) -> anyhow::Result<RawEvt> {
        let isr_id = decode_u32(buf, current_idx).context("Failed to decode 'isr_id' u32 field.")?;
        let name = decode_string(buf, current_idx)?;
        if bytes_left(buf, *current_idx) {
            return Err(anyhow!("Loose bytes at end of 'IsrName' event."));
        }
        Ok(RawEvt::Metadata(RawMetadataEvt::IsrName(Self { isr_id, name })))
    }
}

#[derive(Debug, Clone)]
pub struct IsrEnterEvt {
    pub isr_id: u32,
}

impl IsrEnterEvt {
    fn decode(buf: &[u8], current_idx: &mut usize) -> anyhow::Result<RawEvt> {
        let ts = decode_u64(buf, current_idx)?;
        let isr_id = decode_u32(buf, current_idx).context("Failed to decode 'isr_id' u32 field.")?;
        if bytes_left(buf, *current_idx) {
            return Err(anyhow!("Loose bytes at end of 'IsrEnter' event."));
        }
        Ok(RawEvt::Trace(RawTraceEvt {
            ts,
            kind: RawTraceEvtKind::IsrEnter(Self { isr_id }),
        }))
    }
}

#[derive(Debug, Clone)]
pub struct IsrExitEvt {
    pub isr_id: u32,
}

impl IsrExitEvt {
    fn decode(buf: &[u8], current_idx: &mut usize) -> anyhow::Result<RawEvt> {
        let ts = decode_u64(buf, current_idx)?;
        let isr_id = decode_u32(buf, current_idx).context("Failed to decode 'isr_id' u32 field.")?;
        if bytes_left(buf, *current_idx) {
            return Err(anyhow!("Loose bytes at end of 'IsrExit' event."));
        }
        Ok(RawEvt::Trace(RawTraceEvt {
            ts,
            kind: RawTraceEvtKind::IsrExit(Self { isr_id }),
        }))
    }
}

#[derive(Debug, Clone)]
pub struct QueueCreatedEvt {
    pub queue_id: u32,
}

impl QueueCreatedEvt {
    fn decode(buf: &[u8], current_idx: &mut usize) -> anyhow::Result<RawEvt> {
        let ts = decode_u64(buf, current_idx)?;
        let queue_id = decode_u32(buf, current_idx).context("Failed to decode 'queue_id' u32 field.")?;
        if bytes_left(buf, *current_idx) {
            return Err(anyhow!("Loose bytes at end of 'QueueCreated' event."));
        }
        Ok(RawEvt::Trace(RawTraceEvt {
            ts,
            kind: RawTraceEvtKind::QueueCreated(Self { queue_id }),
        }))
    }
}

#[derive(Debug, Clone)]
pub struct QueueNameEvt {
    pub queue_id: u32,
    pub name: String,
}

impl QueueNameEvt {
    fn decode(buf: &[u8], current_idx: &mut usize) -> anyhow::Result<RawEvt> {
        let queue_id = decode_u32(buf, current_idx).context("Failed to decode 'queue_id' u32 field.")?;
        let name = decode_string(buf, current_idx)?;
        if bytes_left(buf, *current_idx) {
            return Err(anyhow!("Loose bytes at end of 'QueueName' event."));
        }
        Ok(RawEvt::Metadata(RawMetadataEvt::QueueName(Self { queue_id, name })))
    }
}

#[derive(Debug, Clone)]
pub struct QueueKindEvt {
    pub queue_id: u32,
    pub kind: QueueKind,
}

impl QueueKindEvt {
    fn decode(buf: &[u8], current_idx: &mut usize) -> anyhow::Result<RawEvt> {
        let queue_id = decode_u32(buf, current_idx).context("Failed to decode 'queue_id' u32 field.")?;
        let kind = QueueKind::try_from(decode_u8(buf, current_idx).context("Failed to decode 'kind' u8 enum field.")?)
            .context("Failed to decode 'kind' u8 enum field.")?;
        if bytes_left(buf, *current_idx) {
            return Err(anyhow!("Loose bytes at end of 'QueueKind' event."));
        }
        Ok(RawEvt::Metadata(RawMetadataEvt::QueueKind(Self { queue_id, kind })))
    }
}

#[derive(Debug, Clone)]
pub struct QueueSendEvt {
    pub queue_id: u32,
    pub len_after: u32,
}

impl QueueSendEvt {
    fn decode(buf: &[u8], current_idx: &mut usize) -> anyhow::Result<RawEvt> {
        let ts = decode_u64(buf, current_idx)?;
        let queue_id = decode_u32(buf, current_idx).context("Failed to decode 'queue_id' u32 field.")?;
        let len_after = decode_u32(buf, current_idx).context("Failed to decode 'len_after' u32 field.")?;
        if bytes_left(buf, *current_idx) {
            return Err(anyhow!("Loose bytes at end of 'QueueSend' event."));
        }
        Ok(RawEvt::Trace(RawTraceEvt {
            ts,
            kind: RawTraceEvtKind::QueueSend(Self { queue_id, len_after }),
        }))
    }
}

#[derive(Debug, Clone)]
pub struct QueueSendFromIsrEvt {
    pub queue_id: u32,
    pub len_after: u32,
}

impl QueueSendFromIsrEvt {
    fn decode(buf: &[u8], current_idx: &mut usize) -> anyhow::Result<RawEvt> {
        let ts = decode_u64(buf, current_idx)?;
        let queue_id = decode_u32(buf, current_idx).context("Failed to decode 'queue_id' u32 field.")?;
        let len_after = decode_u32(buf, current_idx).context("Failed to decode 'len_after' u32 field.")?;
        if bytes_left(buf, *current_idx) {
            return Err(anyhow!("Loose bytes at end of 'QueueSendFromIsr' event."));
        }
        Ok(RawEvt::Trace(RawTraceEvt {
            ts,
            kind: RawTraceEvtKind::QueueSendFromIsr(Self { queue_id, len_after }),
        }))
    }
}

#[derive(Debug, Clone)]
pub struct QueueOverwriteEvt {
    pub queue_id: u32,
    pub len_after: u32,
}

impl QueueOverwriteEvt {
    fn decode(buf: &[u8], current_idx: &mut usize) -> anyhow::Result<RawEvt> {
        let ts = decode_u64(buf, current_idx)?;
        let queue_id = decode_u32(buf, current_idx).context("Failed to decode 'queue_id' u32 field.")?;
        let len_after = decode_u32(buf, current_idx).context("Failed to decode 'len_after' u32 field.")?;
        if bytes_left(buf, *current_idx) {
            return Err(anyhow!("Loose bytes at end of 'QueueOverwrite' event."));
        }
        Ok(RawEvt::Trace(RawTraceEvt {
            ts,
            kind: RawTraceEvtKind::QueueOverwrite(Self { queue_id, len_after }),
        }))
    }
}

#[derive(Debug, Clone)]
pub struct QueueOverwriteFromIsrEvt {
    pub queue_id: u32,
    pub len_after: u32,
}

impl QueueOverwriteFromIsrEvt {
    fn decode(buf: &[u8], current_idx: &mut usize) -> anyhow::Result<RawEvt> {
        let ts = decode_u64(buf, current_idx)?;
        let queue_id = decode_u32(buf, current_idx).context("Failed to decode 'queue_id' u32 field.")?;
        let len_after = decode_u32(buf, current_idx).context("Failed to decode 'len_after' u32 field.")?;
        if bytes_left(buf, *current_idx) {
            return Err(anyhow!("Loose bytes at end of 'QueueOverwriteFromIsr' event."));
        }
        Ok(RawEvt::Trace(RawTraceEvt {
            ts,
            kind: RawTraceEvtKind::QueueOverwriteFromIsr(Self { queue_id, len_after }),
        }))
    }
}

#[derive(Debug, Clone)]
pub struct QueueReceiveEvt {
    pub queue_id: u32,
    pub len_after: u32,
}

impl QueueReceiveEvt {
    fn decode(buf: &[u8], current_idx: &mut usize) -> anyhow::Result<RawEvt> {
        let ts = decode_u64(buf, current_idx)?;
        let queue_id = decode_u32(buf, current_idx).context("Failed to decode 'queue_id' u32 field.")?;
        let len_after = decode_u32(buf, current_idx).context("Failed to decode 'len_after' u32 field.")?;
        if bytes_left(buf, *current_idx) {
            return Err(anyhow!("Loose bytes at end of 'QueueReceive' event."));
        }
        Ok(RawEvt::Trace(RawTraceEvt {
            ts,
            kind: RawTraceEvtKind::QueueReceive(Self { queue_id, len_after }),
        }))
    }
}

#[derive(Debug, Clone)]
pub struct QueueReceiveFromIsrEvt {
    pub queue_id: u32,
    pub len_after: u32,
}

impl QueueReceiveFromIsrEvt {
    fn decode(buf: &[u8], current_idx: &mut usize) -> anyhow::Result<RawEvt> {
        let ts = decode_u64(buf, current_idx)?;
        let queue_id = decode_u32(buf, current_idx).context("Failed to decode 'queue_id' u32 field.")?;
        let len_after = decode_u32(buf, current_idx).context("Failed to decode 'len_after' u32 field.")?;
        if bytes_left(buf, *current_idx) {
            return Err(anyhow!("Loose bytes at end of 'QueueReceiveFromIsr' event."));
        }
        Ok(RawEvt::Trace(RawTraceEvt {
            ts,
            kind: RawTraceEvtKind::QueueReceiveFromIsr(Self { queue_id, len_after }),
        }))
    }
}

#[derive(Debug, Clone)]
pub struct QueueResetEvt {
    pub queue_id: u32,
}

impl QueueResetEvt {
    fn decode(buf: &[u8], current_idx: &mut usize) -> anyhow::Result<RawEvt> {
        let ts = decode_u64(buf, current_idx)?;
        let queue_id = decode_u32(buf, current_idx).context("Failed to decode 'queue_id' u32 field.")?;
        if bytes_left(buf, *current_idx) {
            return Err(anyhow!("Loose bytes at end of 'QueueReset' event."));
        }
        Ok(RawEvt::Trace(RawTraceEvt {
            ts,
            kind: RawTraceEvtKind::QueueReset(Self { queue_id }),
        }))
    }
}

#[derive(Debug, Clone)]
pub struct CurtaskBlockOnQueuePeekEvt {
    pub queue_id: u32,
    pub ticks_to_wait: u32,
}

impl CurtaskBlockOnQueuePeekEvt {
    fn decode(buf: &[u8], current_idx: &mut usize) -> anyhow::Result<RawEvt> {
        let ts = decode_u64(buf, current_idx)?;
        let queue_id = decode_u32(buf, current_idx).context("Failed to decode 'queue_id' u32 field.")?;
        let ticks_to_wait = decode_u32(buf, current_idx).context("Failed to decode 'ticks_to_wait' u32 field.")?;
        if bytes_left(buf, *current_idx) {
            return Err(anyhow!("Loose bytes at end of 'CurtaskBlockOnQueuePeek' event."));
        }
        Ok(RawEvt::Trace(RawTraceEvt {
            ts,
            kind: RawTraceEvtKind::CurtaskBlockOnQueuePeek(Self {
                queue_id,
                ticks_to_wait,
            }),
        }))
    }
}

#[derive(Debug, Clone)]
pub struct CurtaskBlockOnQueueSendEvt {
    pub queue_id: u32,
    pub ticks_to_wait: u32,
}

impl CurtaskBlockOnQueueSendEvt {
    fn decode(buf: &[u8], current_idx: &mut usize) -> anyhow::Result<RawEvt> {
        let ts = decode_u64(buf, current_idx)?;
        let queue_id = decode_u32(buf, current_idx).context("Failed to decode 'queue_id' u32 field.")?;
        let ticks_to_wait = decode_u32(buf, current_idx).context("Failed to decode 'ticks_to_wait' u32 field.")?;
        if bytes_left(buf, *current_idx) {
            return Err(anyhow!("Loose bytes at end of 'CurtaskBlockOnQueueSend' event."));
        }
        Ok(RawEvt::Trace(RawTraceEvt {
            ts,
            kind: RawTraceEvtKind::CurtaskBlockOnQueueSend(Self {
                queue_id,
                ticks_to_wait,
            }),
        }))
    }
}

#[derive(Debug, Clone)]
pub struct CurtaskBlockOnQueueReceiveEvt {
    pub queue_id: u32,
    pub ticks_to_wait: u32,
}

impl CurtaskBlockOnQueueReceiveEvt {
    fn decode(buf: &[u8], current_idx: &mut usize) -> anyhow::Result<RawEvt> {
        let ts = decode_u64(buf, current_idx)?;
        let queue_id = decode_u32(buf, current_idx).context("Failed to decode 'queue_id' u32 field.")?;
        let ticks_to_wait = decode_u32(buf, current_idx).context("Failed to decode 'ticks_to_wait' u32 field.")?;
        if bytes_left(buf, *current_idx) {
            return Err(anyhow!("Loose bytes at end of 'CurtaskBlockOnQueueReceive' event."));
        }
        Ok(RawEvt::Trace(RawTraceEvt {
            ts,
            kind: RawTraceEvtKind::CurtaskBlockOnQueueReceive(Self {
                queue_id,
                ticks_to_wait,
            }),
        }))
    }
}

#[derive(Debug, Clone)]
pub struct EvtmarkerNameEvt {
    pub evtmarker_id: u32,
    pub name: String,
}

impl EvtmarkerNameEvt {
    fn decode(buf: &[u8], current_idx: &mut usize) -> anyhow::Result<RawEvt> {
        let evtmarker_id = decode_u32(buf, current_idx).context("Failed to decode 'evtmarker_id' u32 field.")?;
        let name = decode_string(buf, current_idx)?;
        if bytes_left(buf, *current_idx) {
            return Err(anyhow!("Loose bytes at end of 'EvtmarkerName' event."));
        }
        Ok(RawEvt::Metadata(RawMetadataEvt::EvtmarkerName(Self { evtmarker_id, name })))
    }
}

#[derive(Debug, Clone)]
pub struct EvtmarkerEvt {
    pub evtmarker_id: u32,
    pub msg: String,
}

impl EvtmarkerEvt {
    fn decode(buf: &[u8], current_idx: &mut usize) -> anyhow::Result<RawEvt> {
        let ts = decode_u64(buf, current_idx)?;
        let evtmarker_id = decode_u32(buf, current_idx).context("Failed to decode 'evtmarker_id' u32 field.")?;
        let msg = decode_string(buf, current_idx)?;
        if bytes_left(buf, *current_idx) {
            return Err(anyhow!("Loose bytes at end of 'Evtmarker' event."));
        }
        Ok(RawEvt::Trace(RawTraceEvt {
            ts,
            kind: RawTraceEvtKind::Evtmarker(Self { evtmarker_id, msg }),
        }))
    }
}

#[derive(Debug, Clone)]
pub struct EvtmarkerBeginEvt {
    pub evtmarker_id: u32,
    pub msg: String,
}

impl EvtmarkerBeginEvt {
    fn decode(buf: &[u8], current_idx: &mut usize) -> anyhow::Result<RawEvt> {
        let ts = decode_u64(buf, current_idx)?;
        let evtmarker_id = decode_u32(buf, current_idx).context("Failed to decode 'evtmarker_id' u32 field.")?;
        let msg = decode_string(buf, current_idx)?;
        if bytes_left(buf, *current_idx) {
            return Err(anyhow!("Loose bytes at end of 'EvtmarkerBegin' event."));
        }
        Ok(RawEvt::Trace(RawTraceEvt {
            ts,
            kind: RawTraceEvtKind::EvtmarkerBegin(Self { evtmarker_id, msg }),
        }))
    }
}

#[derive(Debug, Clone)]
pub struct EvtmarkerEndEvt {
    pub evtmarker_id: u32,
}

impl EvtmarkerEndEvt {
    fn decode(buf: &[u8], current_idx: &mut usize) -> anyhow::Result<RawEvt> {
        let ts = decode_u64(buf, current_idx)?;
        let evtmarker_id = decode_u32(buf, current_idx).context("Failed to decode 'evtmarker_id' u32 field.")?;
        if bytes_left(buf, *current_idx) {
            return Err(anyhow!("Loose bytes at end of 'EvtmarkerEnd' event."));
        }
        Ok(RawEvt::Trace(RawTraceEvt {
            ts,
            kind: RawTraceEvtKind::EvtmarkerEnd(Self { evtmarker_id }),
        }))
    }
}

#[derive(Debug, Clone)]
pub struct ValmarkerNameEvt {
    pub valmarker_id: u32,
    pub name: String,
}

impl ValmarkerNameEvt {
    fn decode(buf: &[u8], current_idx: &mut usize) -> anyhow::Result<RawEvt> {
        let valmarker_id = decode_u32(buf, current_idx).context("Failed to decode 'valmarker_id' u32 field.")?;
        let name = decode_string(buf, current_idx)?;
        if bytes_left(buf, *current_idx) {
            return Err(anyhow!("Loose bytes at end of 'ValmarkerName' event."));
        }
        Ok(RawEvt::Metadata(RawMetadataEvt::ValmarkerName(Self { valmarker_id, name })))
    }
}

#[derive(Debug, Clone)]
pub struct ValmarkerEvt {
    pub valmarker_id: u32,
    pub val: i64,
}

impl ValmarkerEvt {
    fn decode(buf: &[u8], current_idx: &mut usize) -> anyhow::Result<RawEvt> {
        let ts = decode_u64(buf, current_idx)?;
        let valmarker_id = decode_u32(buf, current_idx).context("Failed to decode 'valmarker_id' u32 field.")?;
        let val = decode_s64(buf, current_idx).context("Failed to decode 'val' s64 field.")?;
        if bytes_left(buf, *current_idx) {
            return Err(anyhow!("Loose bytes at end of 'Valmarker' event."));
        }
        Ok(RawEvt::Trace(RawTraceEvt {
            ts,
            kind: RawTraceEvtKind::Valmarker(Self { valmarker_id, val }),
        }))
    }
}

#[derive(Debug, Clone)]
pub struct TaskEvtmarkerNameEvt {
    pub evtmarker_id: u32,
    pub task_id: u32,
    pub name: String,
}

impl TaskEvtmarkerNameEvt {
    fn decode(buf: &[u8], current_idx: &mut usize) -> anyhow::Result<RawEvt> {
        let evtmarker_id = decode_u32(buf, current_idx).context("Failed to decode 'evtmarker_id' u32 field.")?;
        let task_id = decode_u32(buf, current_idx).context("Failed to decode 'task_id' u32 field.")?;
        let name = decode_string(buf, current_idx)?;
        if bytes_left(buf, *current_idx) {
            return Err(anyhow!("Loose bytes at end of 'TaskEvtmarkerName' event."));
        }
        Ok(RawEvt::Metadata(RawMetadataEvt::TaskEvtmarkerName(Self {
            evtmarker_id,
            task_id,
            name,
        })))
    }
}

#[derive(Debug, Clone)]
pub struct TaskEvtmarkerEvt {
    pub evtmarker_id: u32,
    pub msg: String,
}

impl TaskEvtmarkerEvt {
    fn decode(buf: &[u8], current_idx: &mut usize) -> anyhow::Result<RawEvt> {
        let ts = decode_u64(buf, current_idx)?;
        let evtmarker_id = decode_u32(buf, current_idx).context("Failed to decode 'evtmarker_id' u32 field.")?;
        let msg = decode_string(buf, current_idx)?;
        if bytes_left(buf, *current_idx) {
            return Err(anyhow!("Loose bytes at end of 'TaskEvtmarker' event."));
        }
        Ok(RawEvt::Trace(RawTraceEvt {
            ts,
            kind: RawTraceEvtKind::TaskEvtmarker(Self { evtmarker_id, msg }),
        }))
    }
}

#[derive(Debug, Clone)]
pub struct TaskEvtmarkerBeginEvt {
    pub evtmarker_id: u32,
    pub msg: String,
}

impl TaskEvtmarkerBeginEvt {
    fn decode(buf: &[u8], current_idx: &mut usize) -> anyhow::Result<RawEvt> {
        let ts = decode_u64(buf, current_idx)?;
        let evtmarker_id = decode_u32(buf, current_idx).context("Failed to decode 'evtmarker_id' u32 field.")?;
        let msg = decode_string(buf, current_idx)?;
        if bytes_left(buf, *current_idx) {
            return Err(anyhow!("Loose bytes at end of 'TaskEvtmarkerBegin' event."));
        }
        Ok(RawEvt::Trace(RawTraceEvt {
            ts,
            kind: RawTraceEvtKind::TaskEvtmarkerBegin(Self { evtmarker_id, msg }),
        }))
    }
}

#[derive(Debug, Clone)]
pub struct TaskEvtmarkerEndEvt {
    pub evtmarker_id: u32,
}

impl TaskEvtmarkerEndEvt {
    fn decode(buf: &[u8], current_idx: &mut usize) -> anyhow::Result<RawEvt> {
        let ts = decode_u64(buf, current_idx)?;
        let evtmarker_id = decode_u32(buf, current_idx).context("Failed to decode 'evtmarker_id' u32 field.")?;
        if bytes_left(buf, *current_idx) {
            return Err(anyhow!("Loose bytes at end of 'TaskEvtmarkerEnd' event."));
        }
        Ok(RawEvt::Trace(RawTraceEvt {
            ts,
            kind: RawTraceEvtKind::TaskEvtmarkerEnd(Self { evtmarker_id }),
        }))
    }
}

#[derive(Debug, Clone)]
pub struct TaskValmarkerNameEvt {
    pub valmarker_id: u32,
    pub task_id: u32,
    pub name: String,
}

impl TaskValmarkerNameEvt {
    fn decode(buf: &[u8], current_idx: &mut usize) -> anyhow::Result<RawEvt> {
        let valmarker_id = decode_u32(buf, current_idx).context("Failed to decode 'valmarker_id' u32 field.")?;
        let task_id = decode_u32(buf, current_idx).context("Failed to decode 'task_id' u32 field.")?;
        let name = decode_string(buf, current_idx)?;
        if bytes_left(buf, *current_idx) {
            return Err(anyhow!("Loose bytes at end of 'TaskValmarkerName' event."));
        }
        Ok(RawEvt::Metadata(RawMetadataEvt::TaskValmarkerName(Self {
            valmarker_id,
            task_id,
            name,
        })))
    }
}

#[derive(Debug, Clone)]
pub struct TaskValmarkerEvt {
    pub valmarker_id: u32,
    pub val: i64,
}

impl TaskValmarkerEvt {
    fn decode(buf: &[u8], current_idx: &mut usize) -> anyhow::Result<RawEvt> {
        let ts = decode_u64(buf, current_idx)?;
        let valmarker_id = decode_u32(buf, current_idx).context("Failed to decode 'valmarker_id' u32 field.")?;
        let val = decode_s64(buf, current_idx).context("Failed to decode 'val' s64 field.")?;
        if bytes_left(buf, *current_idx) {
            return Err(anyhow!("Loose bytes at end of 'TaskValmarker' event."));
        }
        Ok(RawEvt::Trace(RawTraceEvt {
            ts,
            kind: RawTraceEvtKind::TaskValmarker(Self { valmarker_id, val }),
        }))
    }
}
