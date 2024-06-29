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

// ==== Event Groups ===============================================================================

#[derive(Debug, Clone, Copy)]
pub enum TraceMode {
    Base,
    FreeRTOS,
}

#[derive(Debug, Clone)]
pub enum RawEvt {
    Invalid(InvalidEvt),
    Base(BaseEvt),
    BaseMetadata(BaseMetadataEvt),
    FreeRTOS(FreeRTOSEvt),
    FreeRTOSMetadata(FreeRTOSMetadataEvt),
}

impl RawEvt {
    pub fn ts(&self) -> Option<u64> {
        match self {
            RawEvt::Invalid(e) => e.ts,
            RawEvt::Base(e) => Some(e.ts),
            RawEvt::BaseMetadata(_) => None,
            RawEvt::FreeRTOS(e) => Some(e.ts),
            RawEvt::FreeRTOSMetadata(_) => None,
        }
    }
}

#[derive(Debug, Clone)]
pub struct InvalidEvt {
    pub ts: Option<u64>,
    pub err: Option<Rc<anyhow::Error>>,
}

// ==== Base Event Group ===========================================================================

#[derive(Debug, Clone)]
pub struct BaseEvt {
    pub ts: u64,
    pub kind: BaseEvtKind,
}

#[derive(Debug, Clone)]
pub enum BaseEvtKind {
    CoreId(BaseCoreIdEvt),
    DroppedEvtCnt(BaseDroppedEvtCntEvt),
    IsrEnter(BaseIsrEnterEvt),
    IsrExit(BaseIsrExitEvt),
    Evtmarker(BaseEvtmarkerEvt),
    EvtmarkerBegin(BaseEvtmarkerBeginEvt),
    EvtmarkerEnd(BaseEvtmarkerEndEvt),
    Valmarker(BaseValmarkerEvt),
}

#[derive(Debug, Clone)]
pub enum BaseMetadataEvt {
    TsResolutionNs(BaseTsResolutionNsEvt),
    IsrName(BaseIsrNameEvt),
    EvtmarkerName(BaseEvtmarkerNameEvt),
    ValmarkerName(BaseValmarkerNameEvt),
}

#[derive(Debug, Clone)]
pub struct BaseCoreIdEvt {
    pub core_id: u32,
}

impl BaseCoreIdEvt {
    fn decode(buf: &[u8], current_idx: &mut usize) -> anyhow::Result<RawEvt> {
        let ts = decode_u64(buf, current_idx)?;
        let core_id = decode_u32(buf, current_idx).context("Failed to decode 'core_id' u32 field.")?;
        if bytes_left(buf, *current_idx) {
            return Err(anyhow!("Loose bytes at end of 'CoreId' event."));
        }
        Ok(RawEvt::Base(BaseEvt {
            ts,
            kind: BaseEvtKind::CoreId(Self { core_id }),
        }))
    }
}

#[derive(Debug, Clone)]
pub struct BaseDroppedEvtCntEvt {
    pub cnt: u32,
}

impl BaseDroppedEvtCntEvt {
    fn decode(buf: &[u8], current_idx: &mut usize) -> anyhow::Result<RawEvt> {
        let ts = decode_u64(buf, current_idx)?;
        let cnt = decode_u32(buf, current_idx).context("Failed to decode 'cnt' u32 field.")?;
        if bytes_left(buf, *current_idx) {
            return Err(anyhow!("Loose bytes at end of 'DroppedEvtCnt' event."));
        }
        Ok(RawEvt::Base(BaseEvt {
            ts,
            kind: BaseEvtKind::DroppedEvtCnt(Self { cnt }),
        }))
    }
}

#[derive(Debug, Clone)]
pub struct BaseTsResolutionNsEvt {
    pub ns_per_ts: u64,
}

impl BaseTsResolutionNsEvt {
    fn decode(buf: &[u8], current_idx: &mut usize) -> anyhow::Result<RawEvt> {
        let ns_per_ts = decode_u64(buf, current_idx).context("Failed to decode 'ns_per_ts' u64 field.")?;
        if bytes_left(buf, *current_idx) {
            return Err(anyhow!("Loose bytes at end of 'TsResolutionNs' event."));
        }
        Ok(RawEvt::BaseMetadata(BaseMetadataEvt::TsResolutionNs(Self { ns_per_ts })))
    }
}

#[derive(Debug, Clone)]
pub struct BaseIsrNameEvt {
    pub isr_id: u32,
    pub name: String,
}

impl BaseIsrNameEvt {
    fn decode(buf: &[u8], current_idx: &mut usize) -> anyhow::Result<RawEvt> {
        let isr_id = decode_u32(buf, current_idx).context("Failed to decode 'isr_id' u32 field.")?;
        let name = decode_string(buf, current_idx)?;
        if bytes_left(buf, *current_idx) {
            return Err(anyhow!("Loose bytes at end of 'IsrName' event."));
        }
        Ok(RawEvt::BaseMetadata(BaseMetadataEvt::IsrName(Self { isr_id, name })))
    }
}

#[derive(Debug, Clone)]
pub struct BaseIsrEnterEvt {
    pub isr_id: u32,
}

impl BaseIsrEnterEvt {
    fn decode(buf: &[u8], current_idx: &mut usize) -> anyhow::Result<RawEvt> {
        let ts = decode_u64(buf, current_idx)?;
        let isr_id = decode_u32(buf, current_idx).context("Failed to decode 'isr_id' u32 field.")?;
        if bytes_left(buf, *current_idx) {
            return Err(anyhow!("Loose bytes at end of 'IsrEnter' event."));
        }
        Ok(RawEvt::Base(BaseEvt {
            ts,
            kind: BaseEvtKind::IsrEnter(Self { isr_id }),
        }))
    }
}

#[derive(Debug, Clone)]
pub struct BaseIsrExitEvt {
    pub isr_id: u32,
}

impl BaseIsrExitEvt {
    fn decode(buf: &[u8], current_idx: &mut usize) -> anyhow::Result<RawEvt> {
        let ts = decode_u64(buf, current_idx)?;
        let isr_id = decode_u32(buf, current_idx).context("Failed to decode 'isr_id' u32 field.")?;
        if bytes_left(buf, *current_idx) {
            return Err(anyhow!("Loose bytes at end of 'IsrExit' event."));
        }
        Ok(RawEvt::Base(BaseEvt {
            ts,
            kind: BaseEvtKind::IsrExit(Self { isr_id }),
        }))
    }
}

#[derive(Debug, Clone)]
pub struct BaseEvtmarkerNameEvt {
    pub evtmarker_id: u32,
    pub name: String,
}

impl BaseEvtmarkerNameEvt {
    fn decode(buf: &[u8], current_idx: &mut usize) -> anyhow::Result<RawEvt> {
        let evtmarker_id = decode_u32(buf, current_idx).context("Failed to decode 'evtmarker_id' u32 field.")?;
        let name = decode_string(buf, current_idx)?;
        if bytes_left(buf, *current_idx) {
            return Err(anyhow!("Loose bytes at end of 'EvtmarkerName' event."));
        }
        Ok(RawEvt::BaseMetadata(BaseMetadataEvt::EvtmarkerName(Self { evtmarker_id, name })))
    }
}

#[derive(Debug, Clone)]
pub struct BaseEvtmarkerEvt {
    pub evtmarker_id: u32,
    pub msg: String,
}

impl BaseEvtmarkerEvt {
    fn decode(buf: &[u8], current_idx: &mut usize) -> anyhow::Result<RawEvt> {
        let ts = decode_u64(buf, current_idx)?;
        let evtmarker_id = decode_u32(buf, current_idx).context("Failed to decode 'evtmarker_id' u32 field.")?;
        let msg = decode_string(buf, current_idx)?;
        if bytes_left(buf, *current_idx) {
            return Err(anyhow!("Loose bytes at end of 'Evtmarker' event."));
        }
        Ok(RawEvt::Base(BaseEvt {
            ts,
            kind: BaseEvtKind::Evtmarker(Self { evtmarker_id, msg }),
        }))
    }
}

#[derive(Debug, Clone)]
pub struct BaseEvtmarkerBeginEvt {
    pub evtmarker_id: u32,
    pub msg: String,
}

impl BaseEvtmarkerBeginEvt {
    fn decode(buf: &[u8], current_idx: &mut usize) -> anyhow::Result<RawEvt> {
        let ts = decode_u64(buf, current_idx)?;
        let evtmarker_id = decode_u32(buf, current_idx).context("Failed to decode 'evtmarker_id' u32 field.")?;
        let msg = decode_string(buf, current_idx)?;
        if bytes_left(buf, *current_idx) {
            return Err(anyhow!("Loose bytes at end of 'EvtmarkerBegin' event."));
        }
        Ok(RawEvt::Base(BaseEvt {
            ts,
            kind: BaseEvtKind::EvtmarkerBegin(Self { evtmarker_id, msg }),
        }))
    }
}

#[derive(Debug, Clone)]
pub struct BaseEvtmarkerEndEvt {
    pub evtmarker_id: u32,
}

impl BaseEvtmarkerEndEvt {
    fn decode(buf: &[u8], current_idx: &mut usize) -> anyhow::Result<RawEvt> {
        let ts = decode_u64(buf, current_idx)?;
        let evtmarker_id = decode_u32(buf, current_idx).context("Failed to decode 'evtmarker_id' u32 field.")?;
        if bytes_left(buf, *current_idx) {
            return Err(anyhow!("Loose bytes at end of 'EvtmarkerEnd' event."));
        }
        Ok(RawEvt::Base(BaseEvt {
            ts,
            kind: BaseEvtKind::EvtmarkerEnd(Self { evtmarker_id }),
        }))
    }
}

#[derive(Debug, Clone)]
pub struct BaseValmarkerNameEvt {
    pub valmarker_id: u32,
    pub name: String,
}

impl BaseValmarkerNameEvt {
    fn decode(buf: &[u8], current_idx: &mut usize) -> anyhow::Result<RawEvt> {
        let valmarker_id = decode_u32(buf, current_idx).context("Failed to decode 'valmarker_id' u32 field.")?;
        let name = decode_string(buf, current_idx)?;
        if bytes_left(buf, *current_idx) {
            return Err(anyhow!("Loose bytes at end of 'ValmarkerName' event."));
        }
        Ok(RawEvt::BaseMetadata(BaseMetadataEvt::ValmarkerName(Self { valmarker_id, name })))
    }
}

#[derive(Debug, Clone)]
pub struct BaseValmarkerEvt {
    pub valmarker_id: u32,
    pub val: i64,
}

impl BaseValmarkerEvt {
    fn decode(buf: &[u8], current_idx: &mut usize) -> anyhow::Result<RawEvt> {
        let ts = decode_u64(buf, current_idx)?;
        let valmarker_id = decode_u32(buf, current_idx).context("Failed to decode 'valmarker_id' u32 field.")?;
        let val = decode_s64(buf, current_idx).context("Failed to decode 'val' s64 field.")?;
        if bytes_left(buf, *current_idx) {
            return Err(anyhow!("Loose bytes at end of 'Valmarker' event."));
        }
        Ok(RawEvt::Base(BaseEvt {
            ts,
            kind: BaseEvtKind::Valmarker(Self { valmarker_id, val }),
        }))
    }
}

// ==== FreeRTOS Event Group =======================================================================

#[derive(Debug, Clone)]
pub struct FreeRTOSEvt {
    pub ts: u64,
    pub kind: FreeRTOSEvtKind,
}

#[derive(Debug, Clone)]
pub enum FreeRTOSEvtKind {
    TaskSwitchedIn(FreeRTOSTaskSwitchedInEvt),
    TaskToRdyState(FreeRTOSTaskToRdyStateEvt),
    TaskResumed(FreeRTOSTaskResumedEvt),
    TaskResumedFromIsr(FreeRTOSTaskResumedFromIsrEvt),
    TaskSuspended(FreeRTOSTaskSuspendedEvt),
    CurtaskDelay(FreeRTOSCurtaskDelayEvt),
    CurtaskDelayUntil(FreeRTOSCurtaskDelayUntilEvt),
    TaskPrioritySet(FreeRTOSTaskPrioritySetEvt),
    TaskPriorityInherit(FreeRTOSTaskPriorityInheritEvt),
    TaskPriorityDisinherit(FreeRTOSTaskPriorityDisinheritEvt),
    TaskCreated(FreeRTOSTaskCreatedEvt),
    TaskDeleted(FreeRTOSTaskDeletedEvt),
    QueueCreated(FreeRTOSQueueCreatedEvt),
    QueueSend(FreeRTOSQueueSendEvt),
    QueueSendFromIsr(FreeRTOSQueueSendFromIsrEvt),
    QueueOverwrite(FreeRTOSQueueOverwriteEvt),
    QueueOverwriteFromIsr(FreeRTOSQueueOverwriteFromIsrEvt),
    QueueReceive(FreeRTOSQueueReceiveEvt),
    QueueReceiveFromIsr(FreeRTOSQueueReceiveFromIsrEvt),
    QueueReset(FreeRTOSQueueResetEvt),
    CurtaskBlockOnQueuePeek(FreeRTOSCurtaskBlockOnQueuePeekEvt),
    CurtaskBlockOnQueueSend(FreeRTOSCurtaskBlockOnQueueSendEvt),
    CurtaskBlockOnQueueReceive(FreeRTOSCurtaskBlockOnQueueReceiveEvt),
    TaskEvtmarker(FreeRTOSTaskEvtmarkerEvt),
    TaskEvtmarkerBegin(FreeRTOSTaskEvtmarkerBeginEvt),
    TaskEvtmarkerEnd(FreeRTOSTaskEvtmarkerEndEvt),
    TaskValmarker(FreeRTOSTaskValmarkerEvt),
}

#[derive(Debug, Clone)]
pub enum FreeRTOSMetadataEvt {
    TaskName(FreeRTOSTaskNameEvt),
    TaskIsIdleTask(FreeRTOSTaskIsIdleTaskEvt),
    TaskIsTimerTask(FreeRTOSTaskIsTimerTaskEvt),
    QueueName(FreeRTOSQueueNameEvt),
    QueueKind(FreeRTOSQueueKindEvt),
    TaskEvtmarkerName(FreeRTOSTaskEvtmarkerNameEvt),
    TaskValmarkerName(FreeRTOSTaskValmarkerNameEvt),
}

#[derive(Debug, Clone, Copy)]
pub enum FrQueueKind {
    FrqkQueue,
    FrqkCountingSemphr,
    FrqkBinarySemphr,
    FrqkMutex,
    FrqkRecursiveMutex,
    FrqkQueueSet,
}

impl TryFrom<u8> for FrQueueKind {
    type Error = anyhow::Error;

    fn try_from(value: u8) -> Result<Self, Self::Error> {
        match value {
            0 => Ok(Self::FrqkQueue),
            1 => Ok(Self::FrqkCountingSemphr),
            2 => Ok(Self::FrqkBinarySemphr),
            3 => Ok(Self::FrqkMutex),
            4 => Ok(Self::FrqkRecursiveMutex),
            5 => Ok(Self::FrqkQueueSet),
            _ => Err(anyhow!("Invalid FrQueueKind")),
        }
    }
}

#[derive(Debug, Clone, Copy)]
pub enum FrStreamBufferKind {
    FrsbkStreamBuffer,
    FrsbkMessageBuffer,
}

impl TryFrom<u8> for FrStreamBufferKind {
    type Error = anyhow::Error;

    fn try_from(value: u8) -> Result<Self, Self::Error> {
        match value {
            0 => Ok(Self::FrsbkStreamBuffer),
            1 => Ok(Self::FrsbkMessageBuffer),
            _ => Err(anyhow!("Invalid FrStreamBufferKind")),
        }
    }
}

#[derive(Debug, Clone)]
pub struct FreeRTOSTaskSwitchedInEvt {
    pub task_id: u32,
}

impl FreeRTOSTaskSwitchedInEvt {
    fn decode(buf: &[u8], current_idx: &mut usize) -> anyhow::Result<RawEvt> {
        let ts = decode_u64(buf, current_idx)?;
        let task_id = decode_u32(buf, current_idx).context("Failed to decode 'task_id' u32 field.")?;
        if bytes_left(buf, *current_idx) {
            return Err(anyhow!("Loose bytes at end of 'TaskSwitchedIn' event."));
        }
        Ok(RawEvt::FreeRTOS(FreeRTOSEvt {
            ts,
            kind: FreeRTOSEvtKind::TaskSwitchedIn(Self { task_id }),
        }))
    }
}

#[derive(Debug, Clone)]
pub struct FreeRTOSTaskToRdyStateEvt {
    pub task_id: u32,
}

impl FreeRTOSTaskToRdyStateEvt {
    fn decode(buf: &[u8], current_idx: &mut usize) -> anyhow::Result<RawEvt> {
        let ts = decode_u64(buf, current_idx)?;
        let task_id = decode_u32(buf, current_idx).context("Failed to decode 'task_id' u32 field.")?;
        if bytes_left(buf, *current_idx) {
            return Err(anyhow!("Loose bytes at end of 'TaskToRdyState' event."));
        }
        Ok(RawEvt::FreeRTOS(FreeRTOSEvt {
            ts,
            kind: FreeRTOSEvtKind::TaskToRdyState(Self { task_id }),
        }))
    }
}

#[derive(Debug, Clone)]
pub struct FreeRTOSTaskResumedEvt {
    pub task_id: u32,
}

impl FreeRTOSTaskResumedEvt {
    fn decode(buf: &[u8], current_idx: &mut usize) -> anyhow::Result<RawEvt> {
        let ts = decode_u64(buf, current_idx)?;
        let task_id = decode_u32(buf, current_idx).context("Failed to decode 'task_id' u32 field.")?;
        if bytes_left(buf, *current_idx) {
            return Err(anyhow!("Loose bytes at end of 'TaskResumed' event."));
        }
        Ok(RawEvt::FreeRTOS(FreeRTOSEvt {
            ts,
            kind: FreeRTOSEvtKind::TaskResumed(Self { task_id }),
        }))
    }
}

#[derive(Debug, Clone)]
pub struct FreeRTOSTaskResumedFromIsrEvt {
    pub task_id: u32,
}

impl FreeRTOSTaskResumedFromIsrEvt {
    fn decode(buf: &[u8], current_idx: &mut usize) -> anyhow::Result<RawEvt> {
        let ts = decode_u64(buf, current_idx)?;
        let task_id = decode_u32(buf, current_idx).context("Failed to decode 'task_id' u32 field.")?;
        if bytes_left(buf, *current_idx) {
            return Err(anyhow!("Loose bytes at end of 'TaskResumedFromIsr' event."));
        }
        Ok(RawEvt::FreeRTOS(FreeRTOSEvt {
            ts,
            kind: FreeRTOSEvtKind::TaskResumedFromIsr(Self { task_id }),
        }))
    }
}

#[derive(Debug, Clone)]
pub struct FreeRTOSTaskSuspendedEvt {
    pub task_id: u32,
}

impl FreeRTOSTaskSuspendedEvt {
    fn decode(buf: &[u8], current_idx: &mut usize) -> anyhow::Result<RawEvt> {
        let ts = decode_u64(buf, current_idx)?;
        let task_id = decode_u32(buf, current_idx).context("Failed to decode 'task_id' u32 field.")?;
        if bytes_left(buf, *current_idx) {
            return Err(anyhow!("Loose bytes at end of 'TaskSuspended' event."));
        }
        Ok(RawEvt::FreeRTOS(FreeRTOSEvt {
            ts,
            kind: FreeRTOSEvtKind::TaskSuspended(Self { task_id }),
        }))
    }
}

#[derive(Debug, Clone)]
pub struct FreeRTOSCurtaskDelayEvt {
    pub ticks: u32,
}

impl FreeRTOSCurtaskDelayEvt {
    fn decode(buf: &[u8], current_idx: &mut usize) -> anyhow::Result<RawEvt> {
        let ts = decode_u64(buf, current_idx)?;
        let ticks = decode_u32(buf, current_idx).context("Failed to decode 'ticks' u32 field.")?;
        if bytes_left(buf, *current_idx) {
            return Err(anyhow!("Loose bytes at end of 'CurtaskDelay' event."));
        }
        Ok(RawEvt::FreeRTOS(FreeRTOSEvt {
            ts,
            kind: FreeRTOSEvtKind::CurtaskDelay(Self { ticks }),
        }))
    }
}

#[derive(Debug, Clone)]
pub struct FreeRTOSCurtaskDelayUntilEvt {
    pub time_to_wake: u32,
}

impl FreeRTOSCurtaskDelayUntilEvt {
    fn decode(buf: &[u8], current_idx: &mut usize) -> anyhow::Result<RawEvt> {
        let ts = decode_u64(buf, current_idx)?;
        let time_to_wake = decode_u32(buf, current_idx).context("Failed to decode 'time_to_wake' u32 field.")?;
        if bytes_left(buf, *current_idx) {
            return Err(anyhow!("Loose bytes at end of 'CurtaskDelayUntil' event."));
        }
        Ok(RawEvt::FreeRTOS(FreeRTOSEvt {
            ts,
            kind: FreeRTOSEvtKind::CurtaskDelayUntil(Self { time_to_wake }),
        }))
    }
}

#[derive(Debug, Clone)]
pub struct FreeRTOSTaskPrioritySetEvt {
    pub task_id: u32,
    pub priority: u32,
}

impl FreeRTOSTaskPrioritySetEvt {
    fn decode(buf: &[u8], current_idx: &mut usize) -> anyhow::Result<RawEvt> {
        let ts = decode_u64(buf, current_idx)?;
        let task_id = decode_u32(buf, current_idx).context("Failed to decode 'task_id' u32 field.")?;
        let priority = decode_u32(buf, current_idx).context("Failed to decode 'priority' u32 field.")?;
        if bytes_left(buf, *current_idx) {
            return Err(anyhow!("Loose bytes at end of 'TaskPrioritySet' event."));
        }
        Ok(RawEvt::FreeRTOS(FreeRTOSEvt {
            ts,
            kind: FreeRTOSEvtKind::TaskPrioritySet(Self { task_id, priority }),
        }))
    }
}

#[derive(Debug, Clone)]
pub struct FreeRTOSTaskPriorityInheritEvt {
    pub task_id: u32,
    pub priority: u32,
}

impl FreeRTOSTaskPriorityInheritEvt {
    fn decode(buf: &[u8], current_idx: &mut usize) -> anyhow::Result<RawEvt> {
        let ts = decode_u64(buf, current_idx)?;
        let task_id = decode_u32(buf, current_idx).context("Failed to decode 'task_id' u32 field.")?;
        let priority = decode_u32(buf, current_idx).context("Failed to decode 'priority' u32 field.")?;
        if bytes_left(buf, *current_idx) {
            return Err(anyhow!("Loose bytes at end of 'TaskPriorityInherit' event."));
        }
        Ok(RawEvt::FreeRTOS(FreeRTOSEvt {
            ts,
            kind: FreeRTOSEvtKind::TaskPriorityInherit(Self { task_id, priority }),
        }))
    }
}

#[derive(Debug, Clone)]
pub struct FreeRTOSTaskPriorityDisinheritEvt {
    pub task_id: u32,
    pub priority: u32,
}

impl FreeRTOSTaskPriorityDisinheritEvt {
    fn decode(buf: &[u8], current_idx: &mut usize) -> anyhow::Result<RawEvt> {
        let ts = decode_u64(buf, current_idx)?;
        let task_id = decode_u32(buf, current_idx).context("Failed to decode 'task_id' u32 field.")?;
        let priority = decode_u32(buf, current_idx).context("Failed to decode 'priority' u32 field.")?;
        if bytes_left(buf, *current_idx) {
            return Err(anyhow!("Loose bytes at end of 'TaskPriorityDisinherit' event."));
        }
        Ok(RawEvt::FreeRTOS(FreeRTOSEvt {
            ts,
            kind: FreeRTOSEvtKind::TaskPriorityDisinherit(Self { task_id, priority }),
        }))
    }
}

#[derive(Debug, Clone)]
pub struct FreeRTOSTaskCreatedEvt {
    pub task_id: u32,
}

impl FreeRTOSTaskCreatedEvt {
    fn decode(buf: &[u8], current_idx: &mut usize) -> anyhow::Result<RawEvt> {
        let ts = decode_u64(buf, current_idx)?;
        let task_id = decode_u32(buf, current_idx).context("Failed to decode 'task_id' u32 field.")?;
        if bytes_left(buf, *current_idx) {
            return Err(anyhow!("Loose bytes at end of 'TaskCreated' event."));
        }
        Ok(RawEvt::FreeRTOS(FreeRTOSEvt {
            ts,
            kind: FreeRTOSEvtKind::TaskCreated(Self { task_id }),
        }))
    }
}

#[derive(Debug, Clone)]
pub struct FreeRTOSTaskNameEvt {
    pub task_id: u32,
    pub name: String,
}

impl FreeRTOSTaskNameEvt {
    fn decode(buf: &[u8], current_idx: &mut usize) -> anyhow::Result<RawEvt> {
        let task_id = decode_u32(buf, current_idx).context("Failed to decode 'task_id' u32 field.")?;
        let name = decode_string(buf, current_idx)?;
        if bytes_left(buf, *current_idx) {
            return Err(anyhow!("Loose bytes at end of 'TaskName' event."));
        }
        Ok(RawEvt::FreeRTOSMetadata(FreeRTOSMetadataEvt::TaskName(Self { task_id, name })))
    }
}

#[derive(Debug, Clone)]
pub struct FreeRTOSTaskIsIdleTaskEvt {
    pub task_id: u32,
    pub core_id: u32,
}

impl FreeRTOSTaskIsIdleTaskEvt {
    fn decode(buf: &[u8], current_idx: &mut usize) -> anyhow::Result<RawEvt> {
        let task_id = decode_u32(buf, current_idx).context("Failed to decode 'task_id' u32 field.")?;
        let core_id = decode_u32(buf, current_idx).context("Failed to decode 'core_id' u32 field.")?;
        if bytes_left(buf, *current_idx) {
            return Err(anyhow!("Loose bytes at end of 'TaskIsIdleTask' event."));
        }
        Ok(RawEvt::FreeRTOSMetadata(FreeRTOSMetadataEvt::TaskIsIdleTask(Self { task_id, core_id })))
    }
}

#[derive(Debug, Clone)]
pub struct FreeRTOSTaskIsTimerTaskEvt {
    pub task_id: u32,
}

impl FreeRTOSTaskIsTimerTaskEvt {
    fn decode(buf: &[u8], current_idx: &mut usize) -> anyhow::Result<RawEvt> {
        let task_id = decode_u32(buf, current_idx).context("Failed to decode 'task_id' u32 field.")?;
        if bytes_left(buf, *current_idx) {
            return Err(anyhow!("Loose bytes at end of 'TaskIsTimerTask' event."));
        }
        Ok(RawEvt::FreeRTOSMetadata(FreeRTOSMetadataEvt::TaskIsTimerTask(Self { task_id })))
    }
}

#[derive(Debug, Clone)]
pub struct FreeRTOSTaskDeletedEvt {
    pub task_id: u32,
}

impl FreeRTOSTaskDeletedEvt {
    fn decode(buf: &[u8], current_idx: &mut usize) -> anyhow::Result<RawEvt> {
        let ts = decode_u64(buf, current_idx)?;
        let task_id = decode_u32(buf, current_idx).context("Failed to decode 'task_id' u32 field.")?;
        if bytes_left(buf, *current_idx) {
            return Err(anyhow!("Loose bytes at end of 'TaskDeleted' event."));
        }
        Ok(RawEvt::FreeRTOS(FreeRTOSEvt {
            ts,
            kind: FreeRTOSEvtKind::TaskDeleted(Self { task_id }),
        }))
    }
}

#[derive(Debug, Clone)]
pub struct FreeRTOSQueueCreatedEvt {
    pub queue_id: u32,
}

impl FreeRTOSQueueCreatedEvt {
    fn decode(buf: &[u8], current_idx: &mut usize) -> anyhow::Result<RawEvt> {
        let ts = decode_u64(buf, current_idx)?;
        let queue_id = decode_u32(buf, current_idx).context("Failed to decode 'queue_id' u32 field.")?;
        if bytes_left(buf, *current_idx) {
            return Err(anyhow!("Loose bytes at end of 'QueueCreated' event."));
        }
        Ok(RawEvt::FreeRTOS(FreeRTOSEvt {
            ts,
            kind: FreeRTOSEvtKind::QueueCreated(Self { queue_id }),
        }))
    }
}

#[derive(Debug, Clone)]
pub struct FreeRTOSQueueNameEvt {
    pub queue_id: u32,
    pub name: String,
}

impl FreeRTOSQueueNameEvt {
    fn decode(buf: &[u8], current_idx: &mut usize) -> anyhow::Result<RawEvt> {
        let queue_id = decode_u32(buf, current_idx).context("Failed to decode 'queue_id' u32 field.")?;
        let name = decode_string(buf, current_idx)?;
        if bytes_left(buf, *current_idx) {
            return Err(anyhow!("Loose bytes at end of 'QueueName' event."));
        }
        Ok(RawEvt::FreeRTOSMetadata(FreeRTOSMetadataEvt::QueueName(Self { queue_id, name })))
    }
}

#[derive(Debug, Clone)]
pub struct FreeRTOSQueueKindEvt {
    pub queue_id: u32,
    pub kind: FrQueueKind,
}

impl FreeRTOSQueueKindEvt {
    fn decode(buf: &[u8], current_idx: &mut usize) -> anyhow::Result<RawEvt> {
        let queue_id = decode_u32(buf, current_idx).context("Failed to decode 'queue_id' u32 field.")?;
        let kind =
            FrQueueKind::try_from(decode_u8(buf, current_idx).context("Failed to decode 'kind' u8 enum field.")?)
                .context("Failed to decode 'kind' u8 enum field.")?;
        if bytes_left(buf, *current_idx) {
            return Err(anyhow!("Loose bytes at end of 'QueueKind' event."));
        }
        Ok(RawEvt::FreeRTOSMetadata(FreeRTOSMetadataEvt::QueueKind(Self { queue_id, kind })))
    }
}

#[derive(Debug, Clone)]
pub struct FreeRTOSQueueSendEvt {
    pub queue_id: u32,
    pub len_after: u32,
}

impl FreeRTOSQueueSendEvt {
    fn decode(buf: &[u8], current_idx: &mut usize) -> anyhow::Result<RawEvt> {
        let ts = decode_u64(buf, current_idx)?;
        let queue_id = decode_u32(buf, current_idx).context("Failed to decode 'queue_id' u32 field.")?;
        let len_after = decode_u32(buf, current_idx).context("Failed to decode 'len_after' u32 field.")?;
        if bytes_left(buf, *current_idx) {
            return Err(anyhow!("Loose bytes at end of 'QueueSend' event."));
        }
        Ok(RawEvt::FreeRTOS(FreeRTOSEvt {
            ts,
            kind: FreeRTOSEvtKind::QueueSend(Self { queue_id, len_after }),
        }))
    }
}

#[derive(Debug, Clone)]
pub struct FreeRTOSQueueSendFromIsrEvt {
    pub queue_id: u32,
    pub len_after: u32,
}

impl FreeRTOSQueueSendFromIsrEvt {
    fn decode(buf: &[u8], current_idx: &mut usize) -> anyhow::Result<RawEvt> {
        let ts = decode_u64(buf, current_idx)?;
        let queue_id = decode_u32(buf, current_idx).context("Failed to decode 'queue_id' u32 field.")?;
        let len_after = decode_u32(buf, current_idx).context("Failed to decode 'len_after' u32 field.")?;
        if bytes_left(buf, *current_idx) {
            return Err(anyhow!("Loose bytes at end of 'QueueSendFromIsr' event."));
        }
        Ok(RawEvt::FreeRTOS(FreeRTOSEvt {
            ts,
            kind: FreeRTOSEvtKind::QueueSendFromIsr(Self { queue_id, len_after }),
        }))
    }
}

#[derive(Debug, Clone)]
pub struct FreeRTOSQueueOverwriteEvt {
    pub queue_id: u32,
    pub len_after: u32,
}

impl FreeRTOSQueueOverwriteEvt {
    fn decode(buf: &[u8], current_idx: &mut usize) -> anyhow::Result<RawEvt> {
        let ts = decode_u64(buf, current_idx)?;
        let queue_id = decode_u32(buf, current_idx).context("Failed to decode 'queue_id' u32 field.")?;
        let len_after = decode_u32(buf, current_idx).context("Failed to decode 'len_after' u32 field.")?;
        if bytes_left(buf, *current_idx) {
            return Err(anyhow!("Loose bytes at end of 'QueueOverwrite' event."));
        }
        Ok(RawEvt::FreeRTOS(FreeRTOSEvt {
            ts,
            kind: FreeRTOSEvtKind::QueueOverwrite(Self { queue_id, len_after }),
        }))
    }
}

#[derive(Debug, Clone)]
pub struct FreeRTOSQueueOverwriteFromIsrEvt {
    pub queue_id: u32,
    pub len_after: u32,
}

impl FreeRTOSQueueOverwriteFromIsrEvt {
    fn decode(buf: &[u8], current_idx: &mut usize) -> anyhow::Result<RawEvt> {
        let ts = decode_u64(buf, current_idx)?;
        let queue_id = decode_u32(buf, current_idx).context("Failed to decode 'queue_id' u32 field.")?;
        let len_after = decode_u32(buf, current_idx).context("Failed to decode 'len_after' u32 field.")?;
        if bytes_left(buf, *current_idx) {
            return Err(anyhow!("Loose bytes at end of 'QueueOverwriteFromIsr' event."));
        }
        Ok(RawEvt::FreeRTOS(FreeRTOSEvt {
            ts,
            kind: FreeRTOSEvtKind::QueueOverwriteFromIsr(Self { queue_id, len_after }),
        }))
    }
}

#[derive(Debug, Clone)]
pub struct FreeRTOSQueueReceiveEvt {
    pub queue_id: u32,
    pub len_after: u32,
}

impl FreeRTOSQueueReceiveEvt {
    fn decode(buf: &[u8], current_idx: &mut usize) -> anyhow::Result<RawEvt> {
        let ts = decode_u64(buf, current_idx)?;
        let queue_id = decode_u32(buf, current_idx).context("Failed to decode 'queue_id' u32 field.")?;
        let len_after = decode_u32(buf, current_idx).context("Failed to decode 'len_after' u32 field.")?;
        if bytes_left(buf, *current_idx) {
            return Err(anyhow!("Loose bytes at end of 'QueueReceive' event."));
        }
        Ok(RawEvt::FreeRTOS(FreeRTOSEvt {
            ts,
            kind: FreeRTOSEvtKind::QueueReceive(Self { queue_id, len_after }),
        }))
    }
}

#[derive(Debug, Clone)]
pub struct FreeRTOSQueueReceiveFromIsrEvt {
    pub queue_id: u32,
    pub len_after: u32,
}

impl FreeRTOSQueueReceiveFromIsrEvt {
    fn decode(buf: &[u8], current_idx: &mut usize) -> anyhow::Result<RawEvt> {
        let ts = decode_u64(buf, current_idx)?;
        let queue_id = decode_u32(buf, current_idx).context("Failed to decode 'queue_id' u32 field.")?;
        let len_after = decode_u32(buf, current_idx).context("Failed to decode 'len_after' u32 field.")?;
        if bytes_left(buf, *current_idx) {
            return Err(anyhow!("Loose bytes at end of 'QueueReceiveFromIsr' event."));
        }
        Ok(RawEvt::FreeRTOS(FreeRTOSEvt {
            ts,
            kind: FreeRTOSEvtKind::QueueReceiveFromIsr(Self { queue_id, len_after }),
        }))
    }
}

#[derive(Debug, Clone)]
pub struct FreeRTOSQueueResetEvt {
    pub queue_id: u32,
}

impl FreeRTOSQueueResetEvt {
    fn decode(buf: &[u8], current_idx: &mut usize) -> anyhow::Result<RawEvt> {
        let ts = decode_u64(buf, current_idx)?;
        let queue_id = decode_u32(buf, current_idx).context("Failed to decode 'queue_id' u32 field.")?;
        if bytes_left(buf, *current_idx) {
            return Err(anyhow!("Loose bytes at end of 'QueueReset' event."));
        }
        Ok(RawEvt::FreeRTOS(FreeRTOSEvt {
            ts,
            kind: FreeRTOSEvtKind::QueueReset(Self { queue_id }),
        }))
    }
}

#[derive(Debug, Clone)]
pub struct FreeRTOSCurtaskBlockOnQueuePeekEvt {
    pub queue_id: u32,
    pub ticks_to_wait: u32,
}

impl FreeRTOSCurtaskBlockOnQueuePeekEvt {
    fn decode(buf: &[u8], current_idx: &mut usize) -> anyhow::Result<RawEvt> {
        let ts = decode_u64(buf, current_idx)?;
        let queue_id = decode_u32(buf, current_idx).context("Failed to decode 'queue_id' u32 field.")?;
        let ticks_to_wait = decode_u32(buf, current_idx).context("Failed to decode 'ticks_to_wait' u32 field.")?;
        if bytes_left(buf, *current_idx) {
            return Err(anyhow!("Loose bytes at end of 'CurtaskBlockOnQueuePeek' event."));
        }
        Ok(RawEvt::FreeRTOS(FreeRTOSEvt {
            ts,
            kind: FreeRTOSEvtKind::CurtaskBlockOnQueuePeek(Self {
                queue_id,
                ticks_to_wait,
            }),
        }))
    }
}

#[derive(Debug, Clone)]
pub struct FreeRTOSCurtaskBlockOnQueueSendEvt {
    pub queue_id: u32,
    pub ticks_to_wait: u32,
}

impl FreeRTOSCurtaskBlockOnQueueSendEvt {
    fn decode(buf: &[u8], current_idx: &mut usize) -> anyhow::Result<RawEvt> {
        let ts = decode_u64(buf, current_idx)?;
        let queue_id = decode_u32(buf, current_idx).context("Failed to decode 'queue_id' u32 field.")?;
        let ticks_to_wait = decode_u32(buf, current_idx).context("Failed to decode 'ticks_to_wait' u32 field.")?;
        if bytes_left(buf, *current_idx) {
            return Err(anyhow!("Loose bytes at end of 'CurtaskBlockOnQueueSend' event."));
        }
        Ok(RawEvt::FreeRTOS(FreeRTOSEvt {
            ts,
            kind: FreeRTOSEvtKind::CurtaskBlockOnQueueSend(Self {
                queue_id,
                ticks_to_wait,
            }),
        }))
    }
}

#[derive(Debug, Clone)]
pub struct FreeRTOSCurtaskBlockOnQueueReceiveEvt {
    pub queue_id: u32,
    pub ticks_to_wait: u32,
}

impl FreeRTOSCurtaskBlockOnQueueReceiveEvt {
    fn decode(buf: &[u8], current_idx: &mut usize) -> anyhow::Result<RawEvt> {
        let ts = decode_u64(buf, current_idx)?;
        let queue_id = decode_u32(buf, current_idx).context("Failed to decode 'queue_id' u32 field.")?;
        let ticks_to_wait = decode_u32(buf, current_idx).context("Failed to decode 'ticks_to_wait' u32 field.")?;
        if bytes_left(buf, *current_idx) {
            return Err(anyhow!("Loose bytes at end of 'CurtaskBlockOnQueueReceive' event."));
        }
        Ok(RawEvt::FreeRTOS(FreeRTOSEvt {
            ts,
            kind: FreeRTOSEvtKind::CurtaskBlockOnQueueReceive(Self {
                queue_id,
                ticks_to_wait,
            }),
        }))
    }
}

#[derive(Debug, Clone)]
pub struct FreeRTOSTaskEvtmarkerNameEvt {
    pub evtmarker_id: u32,
    pub task_id: u32,
    pub name: String,
}

impl FreeRTOSTaskEvtmarkerNameEvt {
    fn decode(buf: &[u8], current_idx: &mut usize) -> anyhow::Result<RawEvt> {
        let evtmarker_id = decode_u32(buf, current_idx).context("Failed to decode 'evtmarker_id' u32 field.")?;
        let task_id = decode_u32(buf, current_idx).context("Failed to decode 'task_id' u32 field.")?;
        let name = decode_string(buf, current_idx)?;
        if bytes_left(buf, *current_idx) {
            return Err(anyhow!("Loose bytes at end of 'TaskEvtmarkerName' event."));
        }
        Ok(RawEvt::FreeRTOSMetadata(FreeRTOSMetadataEvt::TaskEvtmarkerName(Self {
            evtmarker_id,
            task_id,
            name,
        })))
    }
}

#[derive(Debug, Clone)]
pub struct FreeRTOSTaskEvtmarkerEvt {
    pub evtmarker_id: u32,
    pub msg: String,
}

impl FreeRTOSTaskEvtmarkerEvt {
    fn decode(buf: &[u8], current_idx: &mut usize) -> anyhow::Result<RawEvt> {
        let ts = decode_u64(buf, current_idx)?;
        let evtmarker_id = decode_u32(buf, current_idx).context("Failed to decode 'evtmarker_id' u32 field.")?;
        let msg = decode_string(buf, current_idx)?;
        if bytes_left(buf, *current_idx) {
            return Err(anyhow!("Loose bytes at end of 'TaskEvtmarker' event."));
        }
        Ok(RawEvt::FreeRTOS(FreeRTOSEvt {
            ts,
            kind: FreeRTOSEvtKind::TaskEvtmarker(Self { evtmarker_id, msg }),
        }))
    }
}

#[derive(Debug, Clone)]
pub struct FreeRTOSTaskEvtmarkerBeginEvt {
    pub evtmarker_id: u32,
    pub msg: String,
}

impl FreeRTOSTaskEvtmarkerBeginEvt {
    fn decode(buf: &[u8], current_idx: &mut usize) -> anyhow::Result<RawEvt> {
        let ts = decode_u64(buf, current_idx)?;
        let evtmarker_id = decode_u32(buf, current_idx).context("Failed to decode 'evtmarker_id' u32 field.")?;
        let msg = decode_string(buf, current_idx)?;
        if bytes_left(buf, *current_idx) {
            return Err(anyhow!("Loose bytes at end of 'TaskEvtmarkerBegin' event."));
        }
        Ok(RawEvt::FreeRTOS(FreeRTOSEvt {
            ts,
            kind: FreeRTOSEvtKind::TaskEvtmarkerBegin(Self { evtmarker_id, msg }),
        }))
    }
}

#[derive(Debug, Clone)]
pub struct FreeRTOSTaskEvtmarkerEndEvt {
    pub evtmarker_id: u32,
}

impl FreeRTOSTaskEvtmarkerEndEvt {
    fn decode(buf: &[u8], current_idx: &mut usize) -> anyhow::Result<RawEvt> {
        let ts = decode_u64(buf, current_idx)?;
        let evtmarker_id = decode_u32(buf, current_idx).context("Failed to decode 'evtmarker_id' u32 field.")?;
        if bytes_left(buf, *current_idx) {
            return Err(anyhow!("Loose bytes at end of 'TaskEvtmarkerEnd' event."));
        }
        Ok(RawEvt::FreeRTOS(FreeRTOSEvt {
            ts,
            kind: FreeRTOSEvtKind::TaskEvtmarkerEnd(Self { evtmarker_id }),
        }))
    }
}

#[derive(Debug, Clone)]
pub struct FreeRTOSTaskValmarkerNameEvt {
    pub valmarker_id: u32,
    pub task_id: u32,
    pub name: String,
}

impl FreeRTOSTaskValmarkerNameEvt {
    fn decode(buf: &[u8], current_idx: &mut usize) -> anyhow::Result<RawEvt> {
        let valmarker_id = decode_u32(buf, current_idx).context("Failed to decode 'valmarker_id' u32 field.")?;
        let task_id = decode_u32(buf, current_idx).context("Failed to decode 'task_id' u32 field.")?;
        let name = decode_string(buf, current_idx)?;
        if bytes_left(buf, *current_idx) {
            return Err(anyhow!("Loose bytes at end of 'TaskValmarkerName' event."));
        }
        Ok(RawEvt::FreeRTOSMetadata(FreeRTOSMetadataEvt::TaskValmarkerName(Self {
            valmarker_id,
            task_id,
            name,
        })))
    }
}

#[derive(Debug, Clone)]
pub struct FreeRTOSTaskValmarkerEvt {
    pub valmarker_id: u32,
    pub val: i64,
}

impl FreeRTOSTaskValmarkerEvt {
    fn decode(buf: &[u8], current_idx: &mut usize) -> anyhow::Result<RawEvt> {
        let ts = decode_u64(buf, current_idx)?;
        let valmarker_id = decode_u32(buf, current_idx).context("Failed to decode 'valmarker_id' u32 field.")?;
        let val = decode_s64(buf, current_idx).context("Failed to decode 'val' s64 field.")?;
        if bytes_left(buf, *current_idx) {
            return Err(anyhow!("Loose bytes at end of 'TaskValmarker' event."));
        }
        Ok(RawEvt::FreeRTOS(FreeRTOSEvt {
            ts,
            kind: FreeRTOSEvtKind::TaskValmarker(Self { valmarker_id, val }),
        }))
    }
}

// ==== Main Decode Function =======================================================================

impl RawEvt {
    pub fn decode(buf: &[u8], mode: TraceMode) -> anyhow::Result<Self> {
        let mut current_idx: usize = 0;
        let id: u8 = decode_u8(buf, &mut current_idx)?;
        match id {
            0x0 => BaseCoreIdEvt::decode(buf, &mut current_idx),
            0x1 => BaseDroppedEvtCntEvt::decode(buf, &mut current_idx),
            0x2 => BaseTsResolutionNsEvt::decode(buf, &mut current_idx),
            0x3 => BaseIsrNameEvt::decode(buf, &mut current_idx),
            0x4 => BaseIsrEnterEvt::decode(buf, &mut current_idx),
            0x5 => BaseIsrExitEvt::decode(buf, &mut current_idx),
            0x6 => BaseEvtmarkerNameEvt::decode(buf, &mut current_idx),
            0x7 => BaseEvtmarkerEvt::decode(buf, &mut current_idx),
            0x8 => BaseEvtmarkerBeginEvt::decode(buf, &mut current_idx),
            0x9 => BaseEvtmarkerEndEvt::decode(buf, &mut current_idx),
            0xA => BaseValmarkerNameEvt::decode(buf, &mut current_idx),
            0xB => BaseValmarkerEvt::decode(buf, &mut current_idx),
            id => match mode {
                TraceMode::Base => Err(anyhow!("Invalid event id 0x{id:X}!")),
                TraceMode::FreeRTOS => match id {
                    0x54 => FreeRTOSTaskSwitchedInEvt::decode(buf, &mut current_idx),
                    0x55 => FreeRTOSTaskToRdyStateEvt::decode(buf, &mut current_idx),
                    0x56 => FreeRTOSTaskResumedEvt::decode(buf, &mut current_idx),
                    0x57 => FreeRTOSTaskResumedFromIsrEvt::decode(buf, &mut current_idx),
                    0x58 => FreeRTOSTaskSuspendedEvt::decode(buf, &mut current_idx),
                    0x59 => FreeRTOSCurtaskDelayEvt::decode(buf, &mut current_idx),
                    0x5A => FreeRTOSCurtaskDelayUntilEvt::decode(buf, &mut current_idx),
                    0x5B => FreeRTOSTaskPrioritySetEvt::decode(buf, &mut current_idx),
                    0x5C => FreeRTOSTaskPriorityInheritEvt::decode(buf, &mut current_idx),
                    0x5D => FreeRTOSTaskPriorityDisinheritEvt::decode(buf, &mut current_idx),
                    0x5E => FreeRTOSTaskCreatedEvt::decode(buf, &mut current_idx),
                    0x5F => FreeRTOSTaskNameEvt::decode(buf, &mut current_idx),
                    0x60 => FreeRTOSTaskIsIdleTaskEvt::decode(buf, &mut current_idx),
                    0x61 => FreeRTOSTaskIsTimerTaskEvt::decode(buf, &mut current_idx),
                    0x62 => FreeRTOSTaskDeletedEvt::decode(buf, &mut current_idx),
                    0x63 => FreeRTOSQueueCreatedEvt::decode(buf, &mut current_idx),
                    0x64 => FreeRTOSQueueNameEvt::decode(buf, &mut current_idx),
                    0x65 => FreeRTOSQueueKindEvt::decode(buf, &mut current_idx),
                    0x66 => FreeRTOSQueueSendEvt::decode(buf, &mut current_idx),
                    0x67 => FreeRTOSQueueSendFromIsrEvt::decode(buf, &mut current_idx),
                    0x68 => FreeRTOSQueueOverwriteEvt::decode(buf, &mut current_idx),
                    0x69 => FreeRTOSQueueOverwriteFromIsrEvt::decode(buf, &mut current_idx),
                    0x6A => FreeRTOSQueueReceiveEvt::decode(buf, &mut current_idx),
                    0x6B => FreeRTOSQueueReceiveFromIsrEvt::decode(buf, &mut current_idx),
                    0x6C => FreeRTOSQueueResetEvt::decode(buf, &mut current_idx),
                    0x6D => FreeRTOSCurtaskBlockOnQueuePeekEvt::decode(buf, &mut current_idx),
                    0x6E => FreeRTOSCurtaskBlockOnQueueSendEvt::decode(buf, &mut current_idx),
                    0x6F => FreeRTOSCurtaskBlockOnQueueReceiveEvt::decode(buf, &mut current_idx),
                    0x7A => FreeRTOSTaskEvtmarkerNameEvt::decode(buf, &mut current_idx),
                    0x7B => FreeRTOSTaskEvtmarkerEvt::decode(buf, &mut current_idx),
                    0x7C => FreeRTOSTaskEvtmarkerBeginEvt::decode(buf, &mut current_idx),
                    0x7D => FreeRTOSTaskEvtmarkerEndEvt::decode(buf, &mut current_idx),
                    0x7E => FreeRTOSTaskValmarkerNameEvt::decode(buf, &mut current_idx),
                    0x7F => FreeRTOSTaskValmarkerEvt::decode(buf, &mut current_idx),
                    id => Err(anyhow!("Invalid event id 0x{id:X}!")),
                },
            },
        }
    }
}
