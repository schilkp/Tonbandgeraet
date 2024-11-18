pub mod decode;
pub mod trace;

use std::collections::{btree_map, BTreeMap};

pub use trace::*;

// == Object Registry ==========================================================

pub trait NewWithId {
    fn new(id: usize) -> Self;
}

pub struct ObjectMap<T>(pub BTreeMap<usize, T>)
where
    T: NewWithId;

impl<T> Default for ObjectMap<T>
where
    T: NewWithId,
{
    fn default() -> Self {
        Self::new()
    }
}

impl<T> ObjectMap<T>
where
    T: NewWithId,
{
    pub fn new() -> Self {
        ObjectMap(BTreeMap::new())
    }

    pub fn get(&self, id: usize) -> Option<&T> {
        self.0.get(&id)
    }

    pub fn get_mut(&mut self, id: usize) -> Option<&mut T> {
        self.0.get_mut(&id)
    }

    pub fn get_or_create(&mut self, id: usize) -> &T {
        self.0.entry(id).or_insert_with(|| T::new(id))
    }

    pub fn get_mut_or_create(&mut self, id: usize) -> &mut T {
        self.0.entry(id).or_insert_with(|| T::new(id))
    }

    pub fn ensure_exists(&mut self, id: usize) {
        self.0.entry(id).or_insert_with(|| T::new(id));
    }
}

impl<'a, T> IntoIterator for &'a ObjectMap<T>
where
    T: NewWithId,
{
    type Item = (&'a usize, &'a T);
    type IntoIter = btree_map::Iter<'a, usize, T>;

    fn into_iter(self) -> Self::IntoIter {
        self.0.iter()
    }
}

impl<'a, T> IntoIterator for &'a mut ObjectMap<T>
where
    T: NewWithId,
{
    type Item = (&'a usize, &'a mut T);
    type IntoIter = btree_map::IterMut<'a, usize, T>;

    fn into_iter(self) -> Self::IntoIter {
        self.0.iter_mut()
    }
}

impl<T> IntoIterator for ObjectMap<T>
where
    T: NewWithId,
{
    type Item = (usize, T);
    type IntoIter = btree_map::IntoIter<usize, T>;

    fn into_iter(self) -> Self::IntoIter {
        self.0.into_iter()
    }
}

// == Timestamped Value ========================================================

pub struct Ts<T> {
    pub ts: u64,
    pub inner: T,
}

impl<T> Ts<T> {
    pub fn new(ts: u64, i: T) -> Ts<T> {
        Ts { ts, inner: i }
    }
}

pub struct Timeseries<T>(pub Vec<Ts<T>>);

impl<T> Default for Timeseries<T> {
    fn default() -> Self {
        Self::new()
    }
}

impl<T> Timeseries<T> {
    pub fn new() -> Self {
        Timeseries(vec![])
    }

    pub fn push(&mut self, ts: u64, t: T) {
        self.0.push(Ts::new(ts, t));
    }
}
